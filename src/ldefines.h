#ifndef LROS_DEFINES_H_
#define LROS_DEFINES_H_

#include <list>
#include <vector>
#include <map>
#include <functional>
#include <type_traits>
#include <iostream>
#include <cassert>

namespace lros
{

// Max Fields Count
static const int kMaxFieldCount = 0xff;
// Max Field ID NO.
static const int kMaxFiledIDNum = 0xff;

//
// LRoleType - indicate the remote role
//
enum LRoleType
{
	kRole_HOST,
	kRole_SLAVE,
};

//
// LRepType
//	- indicate how to replicate objects
//
enum LRepType
{
	kRep_Never,		// never replicate
	kRep_Create,	// replicate only when create, then will tear-off from host
	kRep_Normal,	// replicate when dirty
	kRep_Order		// replicate when dirty, and must make sure the time sequence
};

//
// LTypeID
//	
//
enum LTypeID
{
	l_void	= 0,	// None Type or Class
	l_root	= 0,	// LObject Root Class
	l_bool	= 1,
	l_byte	= 2,
	l_int16	= 3,
	l_int32	= 4,
	l_int64	= 5,
	l_float = 6,
	l_double= 7,
//	l_fix32	= 8,
//	l_fix64 = 9,
	l_string= 0x0a,	// std::string
//	l_binary= 0x0b,
//	l_enum	= 0x0c,
//	l_meta	= 0x0d,	// meta-data
	l_serv	= 0x0e,	// RPC service
	kTypeIDBasicMax	=0x0f,
	kTypeIDUserMax	=0xffff,
};

typedef bool lbool;
typedef char lbyte;
typedef short lint16;
typedef int lint32;
typedef long long lint64;
typedef float lfloat;
typedef double ldouble;
typedef std::string lstring;

template<typename T> struct LTypeTrait 
{ 
	static const int kValue = l_void; 
	static inline T default_value() { return T; }
};
template<> struct LTypeTrait<lbool> 
{ 
	static const int kValue = l_bool; 
	static inline lbool default_value() { return false; }
};
template<> struct LTypeTrait<lbyte> 
{ 
	static const int kValue = l_byte; 
	static inline lbyte default_value() { return 0; }
};
template<> struct LTypeTrait<lint16> 
{ 
	static const int kValue = l_int16; 
	static inline lint16 default_value() { return 0; }
};
template<> struct LTypeTrait<lint32> 
{ 
	static const int kValue = l_int32; 
	static inline lint32 default_value() { return 0; }
};
template<> struct LTypeTrait<lint64> 
{ 
	static const int kValue = l_int64; 
	static inline lint64 default_value() { return 0; }
};
template<> struct LTypeTrait<lfloat> 
{ 
	static const int kValue = l_float; 
	static inline lfloat default_value() { return 0.0f; }
};
template<> struct LTypeTrait<ldouble> 
{ 
	static const int kValue = l_double; 
	static inline ldouble default_value() { return 0.0f; }
};
//template<> struct LType<fix32> { static const int kValue = l_fix32; };
//template<> struct LType<fix64> { static const int kValue = l_fix64; };
template<> struct LTypeTrait<lstring> 
{ 
	static const int kValue = l_string; 
	static inline const lstring& default_value() { static lstring z_str; return z_str; }
};
//template<> struct LType<byte*> { static const int kValue = l_binary; };

class LObject;
class LStream;
class LClass;


class LRefCounter
{
public:
	LRefCounter()
		: m_ref_id(0), m_ref_count(0)
	{
		static int s_counter = 100;
		m_ref_id = m_ref_id++;
	}

	LRefCounter(int _ref_id)
		: m_ref_id(_ref_id), m_ref_count(0)
	{
	}

	static LRefCounter* construct_from_ref_id(int _ref_id)
	{
		assert(_ref_id > 0);
		return new LRefCounter(_ref_id);
	}

	inline int ref_id() { return m_ref_id; }
	inline int ref_count() { return m_ref_count; }

	inline int add() { return ++m_ref_count; }
	inline int release() { return --m_ref_count; }

private:
	int m_ref_id;
	int m_ref_count;
};

template<typename T>
class LRef
{
public:
	~LRef()
	{
		if (m_counter && m_counter->release() == 0)
		{
			assert(m_object);
			delete m_object;
			m_object = NULL;
			delete m_counter;
			m_counter = NULL;
		}
	}

	LRef()
		: m_object(NULL), m_counter(NULL)
	{
	}

	LRef(T* object)
		: m_object(object)
	{
		assert(object);
		m_counter = new LRefCounter;
		m_counter->add();
	}
	LRef(const LRef<T>& rhs)
		: m_object(rhs.m_object), m_counter(rhs.m_counter)
	{
		assert(rhs.m_object && rhs.m_counter);
		m_counter->add();
	}
	LRef<T>& operator=(const LRef<T>& rhs)
	{
		assert(rhs.m_object && rhs.m_counter);
		if (rhs.ref_id() == ref_id()) // same ref
			return *this;
		if (m_counter && m_counter->release() == 0) // release previous
		{
			assert(m_object);
			delete m_object;
			m_object = NULL;
			delete m_counter;
			m_counter = NULL;
		}
		m_object = rhs.m_object;
		m_counter = rhs.m_counter;
		m_counter->add();
		return *this;
	}

	inline void set_ref_id(int _id)
	{

	}

	inline int ref_id() const 
	{
		return m_counter ? m_counter->ref_id() : 0;
	}

	inline T* get()
	{
		return m_object;
	}

	inline const T* get() const
	{
		return m_object;
	}

	inline T* operator->()
	{
		return m_object;
	}

	inline const T* operator->() const
	{
		return m_object;
	}

	inline T& operator*()
	{
		return *m_object;
	}

	inline const T& operator*() const
	{
		return *m_object;
	}

private:
	T* m_object;
	LRefCounter* m_counter;
};

// Meta Class Info
class LClass
{
public:
	typedef std::function<bool(LStream&, const LObject*)> Serializer;
	typedef std::function<bool(LStream&, LObject*)> Deserializer;
	typedef std::function<LObject*()> Creater;
	typedef std::function<void()> Initializer;

	LClass(int class_id, const char* class_name, const LClass* super_class,
		Creater creater, Serializer serializer, Deserializer deserializer, Initializer initializer);

	inline int class_id() const { return m_class_id; }
	inline const char* class_name() const { return m_class_name; }
	inline const LClass* super_class() const { return m_super_class; }

	inline LObject* create_object() { return m_creater(); }
	inline const Serializer& serializer() const { return m_serializer; }
	inline const Deserializer& deserializer() const { return m_deserializer; }

	static const LClass* class_for(int class_id);
	static LObject* create_object(const LClass* cls);
	static LObject* create_object(int class_id);

private:
	static std::map<int, const LClass*> s_class_map;
	int m_class_id;
	const char* m_class_name;
	const LClass* m_super_class;
	Creater m_creater;
	Serializer m_serializer;
	Deserializer m_deserializer;
};


};//lros

#endif //LROS_DEFINES_H_