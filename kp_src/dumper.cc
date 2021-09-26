
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "libprotobuf-mutator/src/libfuzzer/libfuzzer_macro.h"

#include "gen/out.pb.h"

#include "mutator.h"
#include <ctime>

inline std::string slurp(const std::string& path) {
  std::ostringstream buf; 
  std::ifstream input (path.c_str()); 
  buf << input.rdbuf(); 
  return buf.str();
}

extern "C" {
  void *afl_custom_init(void *afl, unsigned int seed);
  size_t afl_custom_fuzz(void *data, unsigned char *buf, size_t buf_size, unsigned char **out_buf, 
                         unsigned char *add_buf, size_t add_buf_size, size_t max_size);
  size_t afl_custom_post_process(void* data, uint8_t *buf, size_t buf_size, uint8_t **out_buf);
  void afl_custom_deinit(void *data);
}

int main(int argc, char *argv[]) {
  menuctf::ChoiceList msg;

  if (argc == 2) {
    std::string data = slurp(argv[1]);
    if(!protobuf_mutator::libfuzzer::LoadProtoInput(true, (const uint8_t *)data.c_str(), data.size(), &msg)) {
      printf("[afl_custom_post_process] LoadProtoInput Error\n");   
      abort();
    }
    
    // 测试变异逻辑
    void* init_data = afl_custom_init(nullptr, time(NULL));
    for(int i = 0; i < 30; i++) {
      uint8_t *out_buf = nullptr;
      size_t new_size = afl_custom_fuzz(init_data, (uint8_t*)data.c_str(), data.size(),
                                                  &out_buf,  nullptr, 0, data.size() + 100);
      uint8_t *new_str = nullptr;
      size_t new_str_size = afl_custom_post_process(init_data, out_buf, new_size, &new_str);
      std::string new_str_str((char*)new_str, new_str_size);
      std::cout << i << ": " << new_str_str << std::endl;
    }
    afl_custom_deinit(init_data);
  } else {
    // alloc 12 "[menuctf::AllocChoice]"
    {
      auto choice = new menuctf::AllocChoice();
      choice->set_size(12);
      choice->set_content("[menuctf::AllocChoice]");

      msg.add_choice()->set_allocated_alloc_choice(choice);
    }

    // update 2 20 "[menuctf::UpdateChoice]"
    {
      auto choice = new menuctf::UpdateChoice();
      choice->set_idx(2);
      choice->set_size(20);
      choice->set_content("[menuctf::UpdateChoice]");

      msg.add_choice()->set_allocated_update_choice(choice);
    }

    // DeleteChoice 3
    {
      auto choice = new menuctf::DeleteChoice();
      choice->set_idx(3);

      msg.add_choice()->set_allocated_delete_choice(choice);
    }

    // ViewChoice 4
    {
      auto choice = new menuctf::ViewChoice();
      choice->set_idx(4);

      msg.add_choice()->set_allocated_view_choice(choice);
    }

    // ExitChoice
    {
      auto choice = new menuctf::ExitChoice();

      msg.add_choice()->set_allocated_exit_choice(choice);
    }

    std::ofstream output_file("output.bin", std::ios::binary);
    // 这里保存的 Serialize 必须使用 Partial 保存，
    msg.SerializePartialToOstream(&output_file);
    output_file.close();
  }

  // std::cout << "msg DebugString: " << msg.DebugString() << std::endl;
  std::stringstream stream;
  ProtoToDataHelper(stream, msg);
  std::cout << stream.str() << std::endl;

  return 0;
}
