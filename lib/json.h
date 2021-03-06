/// \file json.h Holds all JSON-related headers.

#pragma once
#include <string>
#include <deque>
#include <map>
#include <istream>
#include <vector>
#include <set>
#include "socket.h"

/// JSON-related classes and functions
namespace JSON {

  /// Lists all types of JSON::Value.
  enum ValueType {
    EMPTY, BOOL, INTEGER, DOUBLE, STRING, ARRAY, OBJECT
  };

  /// JSON-string-escapes a value
  std::string string_escape(const std::string & val);

  /// A JSON::Value is either a string or an integer, but may also be an object, array or null.
  class Value {
    friend class Iter;
    friend class ConstIter;
    private:
      ValueType myType;
      long long int intVal;
      std::string strVal;
      double dblVal;
      double dblDivider;
      std::deque<Value*> arrVal;
      std::map<std::string, Value*> objVal;
    public:
      //constructors/destructors
      Value();
      ~Value();
      Value(const Value & rhs);
      Value(std::istream & fromstream);
      Value(const std::string & val);
      Value(const char * val);
      Value(long long int val);
      Value(double val);
      Value(bool val);
      //comparison operators
      bool operator==(const Value & rhs) const;
      bool operator!=(const Value & rhs) const;
      bool compareExcept(const Value & rhs, const std::set<std::string> & skip) const;
      bool compareOnly(const Value & rhs, const std::set<std::string> & check) const;
      //assignment operators
      Value & assignFrom(const Value & rhs, const std::set<std::string> & skip);
      Value & operator=(const Value & rhs);
      Value & operator=(const std::string & rhs);
      Value & operator=(const char * rhs);
      Value & operator=(const long long int & rhs);
      Value & operator=(const int & rhs);
      Value & operator=(const double & rhs);
      Value & operator=(const unsigned int & rhs);
      Value & operator=(const bool & rhs);
      //converts to basic types
      operator long long int() const;
      operator std::string() const;
      operator bool() const;
      operator double() const;
      const std::string asString() const;
      const long long int asInt() const;
      const double asDouble() const;
      const bool asBool() const;
      const std::string & asStringRef() const;
      const char * c_str() const;
      //array operator for maps and arrays
      Value & operator[](const std::string i);
      Value & operator[](const char * i);
      Value & operator[](unsigned int i);
      const Value & operator[](const std::string i) const;
      const Value & operator[](const char * i) const;
      const Value & operator[](unsigned int i) const;
      //handy functions and others
      std::string toPacked() const;
      void sendTo(Socket::Connection & socket) const;
      unsigned int packedSize() const;
      void netPrepare();
      std::string & toNetPacked();
      std::string toString() const;
      std::string toPrettyString(int indentation = 0) const;
      void append(const Value & rhs);
      void prepend(const Value & rhs);
      void shrink(unsigned int size);
      void removeMember(const std::string & name);
      void removeMember(const std::deque<Value*>::iterator & it);
      void removeMember(const std::map<std::string, Value*>::iterator & it);
      void removeNullMembers();
      bool isMember(const std::string & name) const;
      bool isInt() const;
      bool isDouble() const;
      bool isString() const;
      bool isBool() const;
      bool isObject() const;
      bool isArray() const;
      bool isNull() const;
      unsigned int size() const;
      void null();
  };

  Value fromDTMI2(std::string & data);
  Value fromDTMI2(const unsigned char * data, unsigned int len, unsigned int & i);
  Value fromDTMI(std::string & data);
  Value fromDTMI(const unsigned char * data, unsigned int len, unsigned int & i);
  Value fromString(const std::string & json);
  Value fromString(const char * data, const uint32_t data_len);
  Value fromFile(std::string filename);
  void fromDTMI2(std::string & data, Value & ret);
  void fromDTMI2(const unsigned char * data, unsigned int len, unsigned int & i, Value & ret);
  void fromDTMI(std::string & data, Value & ret);
  void fromDTMI(const unsigned char * data, unsigned int len, unsigned int & i, Value & ret);

  class Iter {
    public:
      Iter(Value & root);///<Construct from a root Value to iterate over.
      Value & operator*() const;///< Dereferences into a Value reference.
      Value* operator->() const;///< Dereferences into a Value reference.
      operator bool() const;///< True if not done iterating.
      Iter & operator++();///<Go to next iteration.
      const std::string & key() const;///<Return the name of the current indice.
      unsigned int num() const;///<Return the number of the current indice.
      void remove();///<Delete the current indice from the parent JSON::Value.
    private:
      ValueType myType;
      Value * r;
      unsigned int i;
      std::deque<Value*>::iterator aIt;
      std::map<std::string, Value*>::iterator oIt;
  };
  class ConstIter {
    public:
      ConstIter(const Value & root);///<Construct from a root Value to iterate over.
      const Value & operator*() const;///< Dereferences into a Value reference.
      const Value* operator->() const;///< Dereferences into a Value reference.
      operator bool() const;///< True if not done iterating.
      ConstIter & operator++();///<Go to next iteration.
      const std::string & key() const;///<Return the name of the current indice.
      unsigned int num() const;///<Return the number of the current indice.
    private:
      ValueType myType;
      const Value * r;
      unsigned int i;
      std::deque<Value*>::const_iterator aIt;
      std::map<std::string, Value*>::const_iterator oIt;
  };
  #define jsonForEach(val, i) for(JSON::Iter i(val); i; ++i)
  #define jsonForEachConst(val, i) for(JSON::ConstIter i(val); i; ++i)

  template <typename T>
  std::string encodeVector(T begin, T end) {
    std::string result;
    for (T it = begin; it != end; it++) {
      long long int tmp = (*it);
      while (tmp >= 0xFFFF) {
        result += (char)0xFF;
        result += (char)0xFF;
        tmp -= 0xFFFF;
      }
      result += (char)(tmp / 256);
      result += (char)(tmp % 256);
    }
    return result;
  }

  template <typename T>
  void decodeVector(std::string input, T & result) {
    result.clear();
    unsigned int tmp = 0;
    for (int i = 0; i < input.size(); i += 2) {
      unsigned int curLen = (input[i] << 8) + input[i + 1];
      tmp += curLen;
      if (curLen != 0xFFFF) {
        result.push_back(tmp);
        tmp = 0;
      }
    }
  }

  template <typename T>
  std::string encodeVector4(T begin, T end) {
    std::string result;
    for (T it = begin; it != end; it++) {
      long long int tmp = (*it);
      while (tmp >= 0xFFFFFFFF) {
        result += (char)0xFF;
        result += (char)0xFF;
        result += (char)0xFF;
        result += (char)0xFF;
        tmp -= 0xFFFFFFFF;
      }
      result += (char)((tmp & 0xFF000000) >> 24);
      result += (char)((tmp & 0x00FF0000) >> 16);
      result += (char)((tmp & 0x0000FF00) >> 8);
      result += (char)((tmp & 0x000000FF));
    }
    return result;
  }

  template <typename T>
  void decodeVector4(std::string input, T & result) {
    result.clear();
    unsigned int tmp = 0;
    for (int i = 0; i < input.size(); i += 4) {
      unsigned int curLen = (input[i] << 24) + (input[i + 1] << 16) + (input[i + 2] << 8) + (input[i + 3]);
      tmp += curLen;
      if (curLen != 0xFFFFFFFF) {
        result.push_back(tmp);
        tmp = 0;
      }
    }
  }
}
