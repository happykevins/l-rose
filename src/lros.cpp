#include "lros.h"

namespace lros {

LClass::LClass(int class_id, const char* class_name, const LClass* super_class,
			   Creater creater, Serializer serializer, Deserializer deserializer, Initializer initializer)
	: m_class_id(class_id), m_class_name(class_name), m_super_class(super_class),
	m_creater(creater), m_serializer(serializer), m_deserializer(deserializer)
{
	assert((class_id > lros::kTypeIDBasicMax && class_id <= lros::kTypeIDUserMax) || class_id == l_root
		&& "Class ID Error! Make sure - kTypeBasicMax < ClassID < kTypeUserMax !!");
	assert(s_class_map.find(class_id) == s_class_map.end() 
		&& "LClass ID conflict! Make sure this id never used before!");

	s_class_map[class_id] = this;
	initializer();
}

const LClass* LClass::class_for(int class_id)
{
	auto cit = s_class_map.find(class_id);
	if (cit == s_class_map.end())
	{
		return NULL;
	}
	return cit->second;
}

LObject* LClass::create_object(const LClass* cls)
{
	assert(cls);
	return const_cast<LClass*>(cls)->create_object();
}

LObject* LClass::create_object(int class_id)
{
	const LClass* cls = class_for(class_id);
	if (cls == NULL)
		return NULL;
	return create_object(cls);
}


}//lros
