#include "mutator.h"
#include <fstream>
#include <iostream>
#include <unistd.h>

#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/reflection.h"

#include "libprotobuf-mutator/src/libfuzzer/libfuzzer_macro.h"

#include "gen/out.pb.h"

#include <algorithm>
#include <random>
#include <cstdlib>


void ProtoToDataHelper(std::stringstream &out, const google::protobuf::Message &msg) {
  const google::protobuf::Descriptor *desc = msg.GetDescriptor();
  const google::protobuf::Reflection *refl = msg.GetReflection();

  const unsigned fields = desc->field_count();
  // std::cout << msg.DebugString() << std::endl;
  for (unsigned i = 0; i < fields; ++i) {
    const google::protobuf::FieldDescriptor *field = desc->field(i);

    // 对于单个 choice
    if (field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
      // 如果当前是 choice list
      if (field->is_repeated()) {
        const google::protobuf::RepeatedFieldRef<google::protobuf::Message> &ptr = refl->GetRepeatedFieldRef<google::protobuf::Message>(msg, field);
        // 将每个 choice 打出来
        for (const auto &child : ptr) {
          ProtoToDataHelper(out, child);
          out << "\n";
        }
      // 如果当前是某个子 choice
      } else if (refl->HasField(msg, field)) {
        const google::protobuf::Message &child = refl->GetMessage(msg, field);
        ProtoToDataHelper(out, child);
      }
    } 
    // 对于单个 field
    else if (field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_INT32) {
      out << refl->GetInt32(msg, field);
      if(i < fields - 1) 
        out << " ";
    } 
    else if (field->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_STRING) {
      out << refl->GetString(msg, field);
      if(i < fields - 1) 
        out << " ";
    } 
    else {
      abort();
    }

  }
}

// Apparently the textual generator kinda breaks?
DEFINE_BINARY_PROTO_FUZZER(const menuctf::ChoiceList &root) {
  std::stringstream stream;
  ProtoToDataHelper(stream, root);
  std::string data = stream.str();
  std::replace(data.begin(), data.end(), '\n', '|');
  puts(("ProtoToDataHelper: " + data).c_str());
  puts("====================================================================================================");
  // std::string dbg = root.DebugString();
  // std::replace(dbg.begin(), dbg.end(), '\n', ' ');
}

// AFLPlusPlus interface
extern "C" {
  static std::default_random_engine engine_pro;
  static std::uniform_int_distribution<unsigned int> dis(0, UINT32_MAX);

  void *afl_custom_init(void *afl, unsigned int seed) {
    #pragma unused (afl)
    engine_pro.seed(seed);
    return nullptr;
  }
  
  void afl_custom_deinit(void *data) {
    assert(!data);
  }
  
  // afl_custom_fuzz
  size_t afl_custom_fuzz(void *data, unsigned char *buf, size_t buf_size, unsigned char **out_buf, 
                         unsigned char *add_buf, size_t add_buf_size, size_t max_size) {
    #pragma unused (data)
    #pragma unused (add_buf)
    #pragma unused (add_buf_size)
    
    static uint8_t *saved_buf = nullptr;

    assert(buf_size <= max_size);
    
    uint8_t *new_buf = (uint8_t *) realloc((void *)saved_buf, max_size);
    if (!new_buf) {
      *out_buf = buf;
      return buf_size;
    }
    saved_buf = new_buf;

    memcpy(new_buf, buf, buf_size);

    size_t new_size = LLVMFuzzerCustomMutator(
      new_buf,
      buf_size,
      max_size,
      dis(engine_pro)
    );
    *out_buf = new_buf;
    return new_size;
  }

  size_t afl_custom_post_process(void* data, uint8_t *buf, size_t buf_size, uint8_t **out_buf) {
    #pragma unused (data)
    // new_data is never free'd by pre_save_handler
    // I prefer a slow but clearer implementation for now
    
    static uint8_t *saved_buf = NULL;

    menuctf::ChoiceList msg;
    std::stringstream stream;
    // 如果加载成功
    if (protobuf_mutator::libfuzzer::LoadProtoInput(true, buf, buf_size, &msg)) {
      ProtoToDataHelper(stream, msg);
    }
    else {
      // printf("[afl_custom_post_process] LoadProtoInput Error\n");   
      // std::ofstream err_bin("err.bin");
      // err_bin.write((char*)buf, buf_size);

      // abort();

      // 如果加载失败，则返回 Exit Choice
      /// NOTE: 错误的变异 + 错误的 trim 将会导致 post process 加载失败，尤其是 trim 逻辑。
      /// TODO: 由于默认的 trim 会破坏样例，因此需要手动实现一个 trim，这里实现了一个空 trim，不进行任何操作
      ProtoToDataHelper(stream, menuctf::ExitChoice());
    }
    const std::string str = stream.str();

    uint8_t *new_buf = (uint8_t *) realloc((void *)saved_buf, str.size());
    if (!new_buf) {
      *out_buf = buf;
      return buf_size;
    }
    *out_buf = saved_buf = new_buf;

    memcpy((void *)new_buf, str.c_str(), str.size());

    return str.size();
  }

  int32_t  afl_custom_init_trim(void *data, uint8_t *buf, size_t buf_size) {
    /// NOTE: disable trim
    return 0;
  }
  
  size_t afl_custom_trim(void *data, uint8_t **out_buf) {
    /// NOTE: unreachable
    return 0;
  }

}
