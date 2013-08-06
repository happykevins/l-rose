#ifndef LROS_OBJECT_H_
#define LROS_OBJECT_H_

#include "ldefines.h"

namespace lros {
//
// Root Class for LROS Reflection System
//
class LObject
{
public:
	LObject() {};
	virtual ~LObject() {};

	inline static const LClass* l_meta_class() { return &s_meta_class; }

	virtual const LClass* l_class() const;

	bool l_same_class(const LClass* cls) const;
	bool l_same_class(const LObject* obj) const;
	template<typename T>
	bool l_same_class() const { return l_same_class(typename T::l_meta_class()); }

	bool l_instance_of(const LClass* cls) const;
	bool l_instance_of(const LObject* obj) const;
	template<typename T>
	bool l_instance_of() const { return l_instance_of(typename T::l_meta_class()); }

	static void l_static_init() { }
	
	static bool l_serialize(lros::LStream& s, const LObject* obj);
	static bool l_deserialize(lros::LStream& s, LObject* obj);

	static LObject* l_new() { return NULL; }
	virtual void l_init() {};

	template<typename T>
	static inline void __register_fields() {}
	static inline int __fields_reserved() { return -1; }

private:
	static const LClass s_meta_class;
};

// Class field data
template<typename T>
struct LField
{
	typedef std::function<bool(LStream&, const T&)> Serializer;
	typedef std::function<bool(LStream&, T&)> Deserializer;
	typedef std::function<void(T&)> Initializer;
	LField() : field_id(0), field_name(NULL) {}

	int field_id;
	const char* field_name;
	Serializer serializer;
	Deserializer deserializer;
	Initializer initializer;
};

template<typename T>
struct LFieldRegistry
{
	std::vector<lros::LField<T> > field_list;
	std::map<int, lros::LField<T> > field_map;
	int fields_reserved;

	LFieldRegistry() : fields_reserved(-1) {}

	inline lros::LField<T>* get_field(int field_id)
	{
		auto fit = field_map.find(field_id); 
		if (fit == field_map.end()) 
		{ 
			std::cout << "Class:TestMember" << " Error! Invalid Field ID:" << field_id << std::endl; 
			return NULL; 
		} 
		return &fit->second;
	}
};

// LDerivedObject
template<typename T, typename S = lros::LObject>
class LDerivedObject : public S
{
public:
	typedef S LSuperClassType;
	typedef T LClassType;
	typedef LDerivedObject<T, S> LDerivedType;

	static const lros::LClass __meta_class;
	static LFieldRegistry<LClassType> __field_registry;

	virtual const lros::LClass* l_class() const { return &LDerivedType::__meta_class; }
	inline static const lros::LClass* l_meta_class() { return &LDerivedType::__meta_class; }
	static void l_static_init() { __register_fields<LClassType>(); }
	static LClassType* l_new() 
	{ 
		LClassType* obj = new LClassType;
		LClassType::l_constructor(obj);
		return obj; 
	}

	static void l_constructor(LClassType* obj) 
	{ 
		std::cout << __FUNCTION__ << std::endl; 
		for(auto f : LClassType::__field_registry.field_list) 
		{
			f.initializer(*obj);
		} 

		obj->l_init();
	}

	static bool l_serialize(lros::LStream& s, const lros::LObject* obj) 
	{ 
		std::cout << __FUNCTION__ << std::endl; 
		assert(obj->l_same_class<LClassType>() && "Serialized Object Class Must Same!"); 
		const LClassType& dobj = dynamic_cast<const LClassType&>(*obj); 
		
		for(auto f : LClassType::__field_registry.field_list) 
		{ 
			s.write_field_id(f.field_id); 
			f.serializer(s, dobj); 
		} 
		s.write_field_id(-1); 
		return true; 
	} 
	static bool l_deserialize(lros::LStream& s, LObject* obj) 
	{ 
		std::cout << __FUNCTION__ << std::endl; 
		assert(obj->l_same_class<LClassType>() && "Deserialized Object Class Must Same!"); 
		LClassType& dobj = dynamic_cast<LClassType&>(*obj); 
		
		int field_id = -1; 
		for(s.read_field_id(field_id); field_id != -1; s.read_field_id(field_id)) 
		{ 
			auto field = __field_registry.get_field(field_id);
			if (!field) 
			{ 
				std::cout << "Class:TestMember" << " Error! Invalid Field ID:" << field_id << std::endl; 
				return false; 
			} 
			field->deserializer(s, dobj); 
		} 
		return true; 
	}

	static inline int __fields_reserved() 
	{ 
		return LClassType::__field_registry.fields_reserved; 
	}

	template<typename D>
	static inline void __register_fields() 
	{ 
		static_assert(std::is_base_of<lros::LObject, LClassType>::value, "Inherit Path Must include LObject!"); 
		LSuperClassType::__register_fields<D>(); 
		LClassType::__register_fields_impl<D>();
	} 
	static inline int __get_super_fieldid_reserved() 
	{ 
		return LSuperClassType::__fields_reserved(); 
	} 

private:
	static void* operator new(size_t _size)
	{
		return ::operator new(_size);
	}
};

}//lros

#endif//LROS_OBJECT_H_
