
#include <iostream>
#include <sys/time.h>

#include "s3_xml.h"

inline long get_micro_seconds() {
  struct timeval tv;
  if (gettimeofday(&tv, NULL) < 0) {
    return -1;
  }
  return tv.tv_usec;
}

void test() {
  std::string info;

  long t1 = get_micro_seconds();
  S3XmlDoc doc("ListBucketResult");
  doc.AppendToRoot(doc.AllocateNode("Name", "test_bucket"));
  doc.AppendToRoot(doc.AllocateNode("Prefix", "/abc"));
  doc.AppendToRoot(doc.AllocateNode("MaxKeys", std::to_string(1000)));
  doc.AppendToRoot(doc.AllocateNode("Delimiter", "/"));
  doc.AppendToRoot(doc.AllocateNode("ContinuationToken", "/abc/ccc"));
  doc.AppendToRoot(doc.AllocateNode("NextContinuationToken", "/abc/zzz"));
  doc.AppendToRoot(doc.AllocateNode("StartAfter", "/abc/aaa"));
  doc.AppendToRoot(doc.AllocateNode("KeyCount", std::to_string(998)));
  doc.AppendToRoot(doc.AllocateNode("IsTruncated", "true"));

  long t2 = get_micro_seconds();

  for (int i = 0; i < 2000; i++) {
    S3XmlNode* contents = doc.AllocateNode("Contents");
    contents->AppendNode(doc.AllocateNode("Key", "test_object"));
    contents->AppendNode(doc.AllocateNode("LastModified", "1223415445657574"));
    contents->AppendNode(doc.AllocateNode("ETag", "\"" + std::string("abcdefghijklmnopqrstuvwxyz") + "\""));
    contents->AppendNode(doc.AllocateNode("Size", std::to_string(1234567)));
    contents->AppendNode(doc.AllocateNode("StorageClass", "STANDARD"));
    S3XmlNode* owner = doc.AllocateNode("Owner");
    owner->AppendNode(doc.AllocateNode("ID", "xxxxxxxxxxxxx"));
    owner->AppendNode(doc.AllocateNode("DisplayName", "test_user"));
    contents->AppendNode(owner);
    doc.AppendToRoot(contents);
  }

  long t3 = get_micro_seconds();

  doc.ToString(&info);
  long t4 = get_micro_seconds();

  std::cout << t1 << std::endl;
  std::cout << t2 << std::endl;
  std::cout << t3 << std::endl;
  std::cout << t4 << std::endl;
}


int main()
{
  test();
  return 0;
}
