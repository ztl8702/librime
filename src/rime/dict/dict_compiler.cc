//
// Copyright RIME Developers
// Distributed under the BSD License
//
// 2011-11-27 GONG Chen <chen.sst@gmail.com>
//
#include <fstream>
#include <boost/filesystem.hpp>
#include <rime/resource.h>
#include <rime/service.h>
#include <rime/algo/algebra.h>
#include <rime/algo/utilities.h>
#include <rime/dict/dictionary.h>
#include <rime/dict/dict_compiler.h>
#include <rime/dict/dict_settings.h>
#include <rime/dict/entry_collector.h>
#include <rime/dict/preset_vocabulary.h>
#include <rime/dict/prism.h>
#include <rime/dict/table.h>
#include <rime/dict/reverse_lookup_dictionary.h>

namespace rime {

DictCompiler::DictCompiler(Dictionary *dictionary, const string& prefix)
    : dict_name_(dictionary->name()),
      prism_(dictionary->prism()),
      table_(dictionary->table()),
      prefix_(prefix) {
}

static string LocateFile(const string& file_name) {
  the<ResourceResolver> resolver(
      Service::instance().CreateResourceResolver({"build_source", "", ""}));
  return resolver->ResolvePath(file_name).string();
}

bool DictCompiler::Compile(const string &schema_file) {
  LOG(INFO) << "compiling dictionary for " << schema_file <<"\n";
  bool build_table_from_source = true;
  DictSettings settings;
  string dict_file = LocateFile(dict_name_ + ".dict.yaml");
  if (!boost::filesystem::exists(dict_file)) {
    LOG(ERROR) << "source file '" << dict_file << "' does not exist.\n";
    build_table_from_source = false;
  }
  else {
    std::ifstream fin(dict_file.c_str());
    if (!settings.LoadDictHeader(fin)) {
      LOG(ERROR) << "failed to load settings from '" << dict_file << "'.\n";
      return false;
    }
    fin.close();
    LOG(INFO) << "dict name: " << settings.dict_name() <<"\n";
    LOG(INFO) << "dict version: " << settings.dict_version() <<"\n";
  }
  vector<string> dict_files;
  auto tables = settings.GetTables();
  for(auto it = tables->begin(); it != tables->end(); ++it) {
    if (!Is<ConfigValue>(*it))
      continue;
    string dict_name = As<ConfigValue>(*it)->str();
    string dict_file = LocateFile(dict_name + ".dict.yaml");
    if (!boost::filesystem::exists(dict_file)) {
      LOG(ERROR) << "source file '" << dict_file << "' does not exist.\n";
      return false;
    }
    LOG(INFO) << "dict file: " << dict_file <<"\n";
    dict_files.push_back(dict_file);
  }
  LOG(INFO) << "DictCompiler: finished loading files" << "\n";
  uint32_t dict_file_checksum = 0;
  if (!dict_files.empty()) {
    ChecksumComputer cc;
    for (const auto& file_name : dict_files) {
      cc.ProcessFile(file_name);
    }
    if (settings.use_preset_vocabulary()) {
      cc.ProcessFile(PresetVocabulary::DictFilePath());
    }
    dict_file_checksum = cc.Checksum();
  }
  LOG(INFO) << "DictCompiler: finished dict checksum" << "\n";
  
  uint32_t schema_file_checksum =
      schema_file.empty() ? 0 : Checksum(schema_file);
  bool rebuild_table = true;
  bool rebuild_prism = true;
  if (table_->Exists() && table_->Load()) {
    if (!build_table_from_source) {
      dict_file_checksum = table_->dict_file_checksum();
      LOG(INFO) << "reuse existing table: " << table_->file_name();
    }
    if (table_->dict_file_checksum() == dict_file_checksum) {
      rebuild_table = false;
    }
    table_->Close();
  }
  else if (!build_table_from_source) {
    LOG(ERROR) << "neither " << dict_name_ << ".dict.yaml nor "
        << dict_name_ << ".table.bin exists.";
    return false;
  }
  if (prism_->Exists() && prism_->Load()) {
    if (prism_->dict_file_checksum() == dict_file_checksum &&
        prism_->schema_file_checksum() == schema_file_checksum) {
      rebuild_prism = false;
    }
    prism_->Close();
  }
  LOG(INFO) << "DictCompiler: finished schema checksum" << "\n";
  
  LOG(INFO) << dict_file << "[" << dict_files.size() << " file(s)]"
            << " (" << dict_file_checksum << ")\n";
  LOG(INFO) << schema_file << " (" << schema_file_checksum << ")\n";
  LOG(INFO) << "DictCompiler: begin reversedb";
  {
    the<ResourceResolver> resolver(
        Service::instance().CreateResourceResolver(
            {"find_reverse_db", prefix_, ".reverse.bin"}));
    ReverseDb reverse_db(resolver->ResolvePath(dict_name_).string());
    if (!reverse_db.Exists() ||
        !reverse_db.Load() ||
        reverse_db.dict_file_checksum() != dict_file_checksum) {
      rebuild_table = true;
    }
  }
  LOG(INFO) << "DictCompiler: done reversedb\n";
  
  if (build_table_from_source && (options_ & kRebuildTable)) {
    rebuild_table = true;
  }
  if (options_ & kRebuildPrism) {
    rebuild_prism = true;
  }
  LOG(INFO) << "DictCompiler: before BuildTable\n";
  
  if (rebuild_table && !BuildTable(&settings, dict_files, dict_file_checksum))
    return false;
  LOG(INFO) << "DictCompiler: done BuildTable\n";
    
  if (rebuild_prism && !BuildPrism(schema_file,
                                   dict_file_checksum, schema_file_checksum))
    return false;
  LOG(INFO) << "DictCompiler: done BuildPrism\n";
    
  // done!
  return true;
}

static string RelocateToUserDirectory(const string& prefix,
                                      const string& file_name) {
  ResourceResolver resolver(ResourceType{"build_target", prefix, ""});
  resolver.set_root_path(Service::instance().deployer().user_data_dir);
  auto resource_id = boost::filesystem::path(file_name).filename().string();
  return resolver.ResolvePath(resource_id).string();
}

bool DictCompiler::BuildTable(DictSettings* settings,
                              const vector<string>& dict_files,
                              uint32_t dict_file_checksum) {
  LOG(INFO) << "building table...\n";
  table_ = New<Table>(RelocateToUserDirectory(prefix_, table_->file_name()));
  LOG(INFO) << "building table...1\n";

  EntryCollector collector;
  LOG(INFO) << "building table...2\n";
  
  collector.Configure(settings);
  LOG(INFO) << "building table...3\n";
  
  collector.Collect(dict_files);
  LOG(INFO) << "building table...4\n";
  
  if (options_ & kDump) {
    boost::filesystem::path path(table_->file_name());
    path.replace_extension(".txt");
    collector.Dump(path.string());
  }
  LOG(INFO) << "building table...5\n";
  
  Vocabulary vocabulary;
  LOG(INFO) << "before building .table.bin...\n";
  
  // build .table.bin
  {
    map<string, SyllableId> syllable_to_id;
    SyllableId syllable_id = 0;
    for (const auto& s : collector.syllabary) {
      syllable_to_id[s] = syllable_id++;
    }
    LOG(INFO) << "syllabary id max=" << syllable_id<<"...\n";
    for (RawDictEntry& r : collector.entries) {
      Code code;
      for (const auto& s : r.raw_code) {
        code.push_back(syllable_to_id[s]);
      }
      DictEntryList* ls = vocabulary.LocateEntries(code);
      if (!ls) {
        LOG(ERROR) << "Error locating entries in vocabulary.\n";
        continue;
      }
      auto e = New<DictEntry>();
      e->code.swap(code);
      e->text.swap(r.text);
      e->weight = r.weight;
      ls->push_back(e);
      //LOG(INFO)<<e->text <<"\n";
    }
    LOG(INFO)<<"before sort homophones"<<"\n";
    
    if (settings->sort_order() != "original") {
      vocabulary.SortHomophones();
    }
    LOG(INFO)<<"done sort homophones"<<"\n";
    
    table_->Remove();
    LOG(INFO)<<"done sort homophones"<<"\n";
    
    if (!table_->Build(collector.syllabary, vocabulary, collector.num_entries,
                       dict_file_checksum) ||
        !table_->Save()) {
      return false;
    }
  }
  LOG(INFO)<<"before building .reverse.bin"<<"\n";
  // build .reverse.bin
  ReverseDb reverse_db(RelocateToUserDirectory(prefix_,
                                               dict_name_ + ".reverse.bin"));
  if (!reverse_db.Build(settings,
                        collector.syllabary,
                        vocabulary,
                        collector.stems,
                        dict_file_checksum)) {
    LOG(ERROR) << "error building reversedb.";
    return false;
  }
  return true;
}

bool DictCompiler::BuildPrism(const string &schema_file,
                              uint32_t dict_file_checksum,
                              uint32_t schema_file_checksum) {
  LOG(INFO) << "building prism...";
  prism_ = New<Prism>(RelocateToUserDirectory(prefix_, prism_->file_name()));

  // get syllabary from table
  Syllabary syllabary;
  if (!table_->Load() || !table_->GetSyllabary(&syllabary) || syllabary.empty())
    return false;
  // apply spelling algebra
  Script script;
  if (!schema_file.empty()) {
    Config config;
    if (!config.LoadFromFile(schema_file)) {
      LOG(ERROR) << "error loading prism definition from " << schema_file;
      return false;
    }
    Projection p;
    auto algebra = config.GetList("speller/algebra");
    if (algebra && p.Load(algebra)) {
      for (const auto& x : syllabary) {
        script.AddSyllable(x);
      }
      if (!p.Apply(&script)) {
        script.clear();
      }
    }
  }
  if ((options_ & kDump) && !script.empty()) {
    boost::filesystem::path path(prism_->file_name());
    path.replace_extension(".txt");
    script.Dump(path.string());
  }
  // build .prism.bin
  {
    prism_->Remove();
    if (!prism_->Build(syllabary, script.empty() ? NULL : &script,
                       dict_file_checksum, schema_file_checksum) ||
        !prism_->Save()) {
      return false;
    }
  }
  return true;
}

}  // namespace rime
