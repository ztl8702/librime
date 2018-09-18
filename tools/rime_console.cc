//
// Copyright RIME Developers
// Distributed under the BSD License
//
// 2011-04-24 GONG Chen <chen.sst@gmail.com>
//
#include <cstring>
#include <iostream>
#include <rime/candidate.h>
#include <rime/common.h>
#include <rime/composition.h>
#include <rime/context.h>
#include <rime/deployer.h>
#include <rime/engine.h>
#include <rime/key_event.h>
#include <rime/menu.h>
#include <rime/schema.h>
#include <rime/setup.h>
#include <rime/dict/dictionary.h>
#include <rime/dict/dict_compiler.h>
#include <rime/lever/deployment_tasks.h>
#include <emscripten/bind.h>
#include <sstream>
#include <locale>
#include <codecvt>
#include <map>

using namespace rime;
using namespace emscripten;
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> ws_converter;

void callback(string str){
   val window = val::global("window");
   std::wstring wide = ws_converter.from_bytes(str);
   window.call<void>("rime_callback", wide);
}

class RimeConsole {
 public:
  RimeConsole() : interactive_(false),
                  engine_(Engine::Create()) {
    conn_ = engine_->sink().connect(
        [this](const string& x) { OnCommit(x); });
  }
  ~RimeConsole() {
    conn_.disconnect();
  }

  void OnCommit(const string &commit_text) {
    if (interactive_) {
    //  std::cout << "commit : [" << commit_text << "]" << std::endl;
      callback("{ \"type\":\"commit\",\"text\" : \"" + commit_text + "\"}" );
    }
    else {
      std::cout << commit_text << std::endl;
    }
  }

  void PrintComposition(const Context *ctx) {
    if (!ctx || !ctx->IsComposing())
      return;
    std::stringstream ss;
    ss << "{ \"type\":\"composing\",";
    ss << "\"input\" : \"" << ctx->input() << "\"," << std::endl;
    const Composition &comp = ctx->composition();
    if (comp.empty())
      return;
    ss << " \"comp\" : \"" << comp.GetDebugText() << "\"," << std::endl;
    const Segment &current(comp.back());
    if (!current.menu)
      return;
    int page_size = engine_->schema()->page_size();
    int page_no = current.selected_index / page_size;
    the<Page> page(current.menu->CreatePage(page_size, page_no));
    if (!page)
      return;
    ss << "\"page_no\": " << page_no <<","
              << " \"index\": " << current.selected_index << "," << std::endl;
    ss << "\"cand\":[";
    int i = 0;
    for (const an<Candidate> &cand : page->candidates) {
      ++i;
      ss <<  " {\"text\": \"";
      ss << cand->text();
      ss << "\",";
      if (!cand->comment().empty())
        ss << "\"comment\": \" " << cand->comment()<<"\",";
      ss<< " \"quality\":" << cand->quality();
      ss <<"}";
      if (i<page->candidates.size()) {
        ss<<",";
      }
      ss<<std::endl;
    }
    ss<<"] }";
    callback(ss.str());
  }

  void ProcessLine(const string &line) {
    KeySequence keys;
    if (!keys.Parse(line)) {
      LOG(ERROR) << "error parsing input: '" << line << "'";
      return;
    }
    for (const KeyEvent &key : keys) {
      engine_->ProcessKey(key);
    }
    Context *ctx = engine_->active_context();
    if (interactive_) {
      PrintComposition(ctx);
    }
    else {
      if (ctx && ctx->IsComposing()) {
        ctx->Commit();
      }
    }
  }

  void set_interactive(bool interactive) { interactive_ = interactive; }
  bool interactive() const { return interactive_; }

 private:
  bool interactive_;
  the<Engine> engine_;
  connection conn_;
};

RimeConsole* gconsole;
int init_rime(){
  // initialize la Rime
  SetupLogging("rime.console");
  LoadModules(kDefaultModules);

  Deployer deployer;
  InstallationUpdate installation;
  if (!installation.Run(&deployer)) {
    std::cerr << "failed to initialize installation." << std::endl;
    return 1;
  }
  std::cerr << "initializing...";
  WorkspaceUpdate workspace_update;
  if (!workspace_update.Run(&deployer)) {
    std::cerr << "failure!" << std::endl;
    return 1;
  }
  std::cout << "ready." << std::endl;
  gconsole = new RimeConsole();
  gconsole->set_interactive(true);
  gconsole->ProcessLine("abc abc {enter}");
}

void process_input(string inputString) {
  std::cout<<"Received input: "<<inputString<<"\n";
  gconsole->ProcessLine(inputString);
  std::cout<<"done processsing"<<"\n";
  
}

int main() {
  std::cout<<"Hello World"<<"\n";
}

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("init", &init_rime);
    emscripten::function("input", &process_input);
}