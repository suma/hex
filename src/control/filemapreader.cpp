
#include "filemapreader.h"


FileMapReader::FileMapReader(QFile *file, size_t min_size)
  : file_(file)
  , ptr_(NULL)
  , buffer_size_(min_size * 0x10)
  , min_size_(min_size)
  , offset_(0)
  , size_(0)
  , iterate_(0)
{
  Q_ASSERT(file != NULL);
  // TODO: increment reference to QFile
}

FileMapReader::~FileMapReader()
{
  // TODO: decrement reference to QFile
  if (ptr_ != NULL) {
    file_->unmap(ptr_);
    ptr_ = NULL;
  }
}

quint64 FileMapReader::length() const
{
  return file_->size();
}

uchar *FileMapReader::get() const
{
  Q_ASSERT(ptr_ != NULL);
  return ptr_ + iterate_;
}

bool FileMapReader::seek(quint64 pos)
{
  Q_ASSERT(pos <= length());
  if (pos > length()) {
    // TODO: error
    return false;
  }

  if (ptr_ != NULL) {
    file_->unmap(ptr_);
  }

  offset_ = pos;
  iterate_ = 0;
  size_ = qMin(length() - offset_, static_cast<quint64>(buffer_size_));
  ptr_ = file_->map(offset_, size_);

  return ptr_ != NULL;
}

void FileMapReader::operator+=(size_t size)
{
  iterate_ += size;

  const quint64 pos = offset_ + iterate_;

  if (pos > length()) {
    return;
  }

  if (iterate_ + min_size_ > size_) {
    seek(pos);
  }
}
