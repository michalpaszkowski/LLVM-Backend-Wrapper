#pragma once

#include <string>

// Shared interface for the resulting object.
struct IBackend_Result {
  IBackend_Result(std::string ResultObject, std::string Errors)
      : resultObject(ResultObject), errors(Errors) {}

  // Returns the pointer to the object buffer.
  virtual const char *GetObject() { return resultObject.data(); }
  // Returns the size in bytes of the object buffer.
  virtual const size_t GetObjectSize() { return resultObject.size(); }

  // Returns the pointer to the error log string.
  virtual const char *GetErrorLog() { return errors.c_str(); }

  // Releases the resulting object.
  virtual void Release() { delete this; }

protected:
  virtual ~IBackend_Result() {}

private:
  std::string resultObject;
  std::string errors;
};

extern "C" IBackend_Result *generateTargetCode(const char *BitcodeBuffer,
                                               size_t BitcodeSize);