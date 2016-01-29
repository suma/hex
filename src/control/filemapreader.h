#pragma once

#include <QFile>

class FileMapReader
{
private:
  QFile *file_;
  uchar *ptr_;
  size_t buffer_size_;  // minimum block size
  size_t min_size_;  // minimum block size

  // mapped offset, size
  quint64 offset_;
  quint64 size_;

  // offset
  quint64 iterate_;

public:
  FileMapReader(QFile *file, size_t min_size);
  ~FileMapReader();

  quint64 length() const;
  uchar *get() const;
  bool seek(quint64 pos);
  void operator +=(size_t size);
};
