#ifndef LROS_STREAM_H_
#define LROS_STREAM_H_

#include "ldefines.h"

#include <sstream>
#include <fstream>

namespace lros
{

class LStream
{
public:
	virtual ~LStream() {}

	// write functions
	virtual bool write_type_id(const int& type_id) = 0;
	virtual bool write_field_id(const int& field_id) = 0;
	virtual bool write_ref_id(const int& ref_id) = 0;
	virtual bool write_object(const LObject* o) = 0;

	virtual bool write_bool(const lbool& v) = 0;
	virtual bool write_byte(const lbyte& v) = 0;
	virtual bool write_int16(const lint16& v) = 0;
	virtual bool write_int32(const lint32& v) = 0;
	virtual bool write_int64(const lint64& v) = 0;
	virtual bool write_float(const lfloat& v) = 0;
	virtual bool write_double(const ldouble& v) = 0;
	virtual bool write_string(const lstring& v) = 0;
	virtual bool write_bytes(const lbyte* buf, size_t len) = 0;

	// read functions
	virtual bool read_type_id(int& type_id) = 0;
	virtual bool read_field_id(int& field_id) = 0;
	virtual bool read_ref_id(int& ref_id) = 0;
	virtual bool read_object(LObject* o) = 0;

	virtual bool read_bool(lbool& v) = 0;
	virtual bool read_byte(lbyte& v) = 0;
	virtual bool read_int16(lint16& v) = 0;
	virtual bool read_int32(lint32& v) = 0;
	virtual bool read_int64(lint64& v) = 0;
	virtual bool read_float(lfloat& v) = 0;
	virtual bool read_double(ldouble& v) = 0;
	virtual bool read_string(lstring& v) = 0;
	virtual bool read_bytes(lbyte* buf, size_t len) = 0;

	//////////////////////////////////////////////////////////////////////////
	template<typename T> inline bool write(const T& v);
	template<typename T> inline bool read(T& v);

	template<> inline bool write(const lbool& v) { return write_bool(v); }
	template<> inline bool write(const lbyte& v) { return write_byte(v); }
	template<> inline bool write(const lint16& v) { return write_int16(v); }
	template<> inline bool write(const lint32& v) { return write_int32(v); }
	template<> inline bool write(const lint64& v) { return write_int64(v); }
	template<> inline bool write(const lfloat& v) { return write_float(v); }
	template<> inline bool write(const ldouble& v) { return write_double(v); }
	template<> inline bool write(const lstring& v) { return write_string(v); }
	template<> inline bool write(const LObject& v) { return write_object(&v); }

	template<> inline bool read(lbool& v) { return read_bool(v); }
	template<> inline bool read(lbyte& v) { return read_byte(v); }
	template<> inline bool read(lint16& v) { return read_int16(v); }
	template<> inline bool read(lint32& v) { return read_int32(v); }
	template<> inline bool read(lint64& v) { return read_int64(v); }
	template<> inline bool read(lfloat& v) { return read_float(v); }
	template<> inline bool read(ldouble& v) { return read_double(v); }
	template<> inline bool read(lstring& v) { return read_string(v); }
	template<> inline bool read(LObject& v) { return read_object(&v); }

	template<typename T>
	inline bool write(const LRef<T>& v)
	{
		write_ref_id(v.ref_id());
		if (v.ref_id() == 0)
			return true;
		return write_object(v.get());
	}

	template<typename T>
	inline bool read(LRef<T>& v)
	{
		int ref_id = 0;
		read_ref_id(ref_id);
		v.set_ref_id(ref_id);
		if (ref_id == 0)
			return true;
		if (!v.get()) // TODO: get object from ref_cache
			v = LRef<T>(typename T::l_new());
		return read_object(v.get());
	}
};

class LFStream : public LStream
{
public:
	static const int kMaxStringLength = 1023;

	// write functions
	virtual bool write_type_id(const int& type_id) 
	{ 
		return write_int32(type_id); 
	}
	virtual bool write_field_id(const int& field_id) 
	{ 
		return write_int32(field_id); 
	}
	virtual bool write_ref_id(const int& ref_id) 
	{ 
		return write_int32(ref_id); 
	}
	virtual bool write_object(const LObject* o) 
	{
		o->l_class()->serializer()(*this, o);
		return true;
	}

	virtual bool write_bool(const lbool& v) 
	{
		return write_byte(v ? 1 : 0);
	}
	virtual bool write_byte(const lbyte& v) 
	{
		return write_bytes((const char*)&v, sizeof(lbyte));
	}
	virtual bool write_int16(const lint16& v) 
	{
		return write_bytes((const char*)&v, sizeof(lint16));
	}
	virtual bool write_int32(const lint32& v) 
	{
		return write_bytes((const char*)&v, sizeof(lint32));
	}
	virtual bool write_int64(const lint64& v) 
	{
		return write_bytes((const char*)&v, sizeof(lint64));
	}
	virtual bool write_float(const lfloat& v) 
	{
		return write_bytes((const char*)&v, sizeof(lfloat));
	}
	virtual bool write_double(const ldouble& v) 
	{
		return write_bytes((const char*)&v, sizeof(ldouble));
	}
	virtual bool write_string(const lstring& v) 
	{
		size_t write_len = v.size() < kMaxStringLength ? v.size() : kMaxStringLength;
		write_int16(write_len);
		write_bytes(v.c_str(), write_len);
		return true;
	}
	virtual bool write_bytes(const lbyte* buf, size_t len) 
	{
		m_fstream->write(buf, len);
		return true;
	}

	// read functions
	virtual bool read_type_id(int& type_id) 
	{ 
		return read_int32(type_id); 
	}
	virtual bool read_field_id(int& field_id) 
	{ 
		return read_int32(field_id);
	}
	virtual bool read_ref_id(int& ref_id) 
	{ 
		return read_int32(ref_id);
	}
	virtual bool read_object(LObject* o) 
	{
		o->l_class()->deserializer()(*this, o);
		return true;
	}

	virtual bool read_bool(lbool& v) 
	{
		lbyte b = 0;
		if (!read_byte(b))
			return false;
		v = b == 0 ? false : true;
		return true;
	}
	virtual bool read_byte(lbyte& v) 
	{
		return read_bytes((lbyte*)&v, sizeof(lbyte));
	}
	virtual bool read_int16(lint16& v) 
	{
		return read_bytes((lbyte*)&v, sizeof(lint16));
	}
	virtual bool read_int32(lint32& v) 
	{
		return read_bytes((lbyte*)&v, sizeof(lint32));
	}
	virtual bool read_int64(lint64& v) 
	{
		return read_bytes((lbyte*)&v, sizeof(lint64));
	}
	virtual bool read_float(lfloat& v) 
	{
		return read_bytes((lbyte*)&v, sizeof(lfloat));
	}
	virtual bool read_double(ldouble& v) 
	{
		return read_bytes((lbyte*)&v, sizeof(ldouble));
	}
	virtual bool read_string(lstring& v) 
	{
		lint16 len = 0;
		if (!read_int16(len))
			return false;

		if (!read_bytes(m_buffer, len))
			return false;

		m_buffer[len] = '\0';
		v.assign(m_buffer, m_buffer + len);

		return true;
	}
	virtual bool read_bytes(lbyte* buf, size_t len) 
	{
		m_fstream->read(buf, len);
		return true;
	}



	LFStream() : m_fstream(NULL)
	{
	}

	virtual ~LFStream()
	{
		if (m_fstream)
		{
			delete m_fstream;
			m_fstream = NULL;
		}
	}

	std::fstream* m_fstream;
	char m_buffer[kMaxStringLength + 1];
};


};

#endif //LROS_STREAM_H_