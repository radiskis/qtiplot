#ifndef REF_H
#define REF_H

#include "ObjectId.h"
#include "ObjectRegistry.h"
#include <type_traits>

template <typename T>
class Ref
{
public:
	Ref() : m_id(NullObjectId) {}

	Ref(ObjectId id) : m_id(id) {}

	Ref(T *ptr) : m_id(NullObjectId)
	{
		if (ptr)
			m_id = ObjectRegistry::instance()->registerObject(ptr);
	}

	Ref(std::nullptr_t) : m_id(NullObjectId) {}
	Ref(int val) : m_id(val == 0 ? NullObjectId : static_cast<ObjectId>(val)) {}

	Ref(const Ref &other) = default;
	Ref(Ref &&other) noexcept = default;
	Ref& operator=(const Ref &other) = default;
	Ref& operator=(Ref &&other) noexcept = default;

	Ref& operator=(std::nullptr_t)
	{
		m_id = NullObjectId;
		return *this;
	}

	Ref& operator=(int val)
	{
		m_id = (val == 0 ? NullObjectId : static_cast<ObjectId>(val));
		return *this;
	}

	Ref& operator=(T *ptr)
	{
		if (ptr)
			m_id = ObjectRegistry::instance()->registerObject(ptr);
		else
			m_id = NullObjectId;
		return *this;
	}

	Ref& operator=(ObjectId id)
	{
		m_id = id;
		return *this;
	}

	T* get() const
	{
		if (m_id == NullObjectId)
			return nullptr;
		return ObjectRegistry::instance()->resolve<T>(m_id);
	}

	T* operator->() const
	{
		return get();
	}

	T& operator*() const
	{
		return *get();
	}

	operator T*() const
	{
		return get();
	}

	explicit operator bool() const
	{
		return get() != nullptr;
	}

	bool isValid() const
	{
		return get() != nullptr;
	}

	ObjectId id() const
	{
		return m_id;
	}

	void reset()
	{
		m_id = NullObjectId;
	}

	template <typename U>
	Ref<U> as() const
	{
		T *ptr = get();
		if (!ptr)
			return Ref<U>();

		if constexpr (std::is_base_of_v<QObject, T> && std::is_base_of_v<QObject, U>) {
			if (qobject_cast<U*>(ptr))
				return Ref<U>(m_id);
		} else {
			if (dynamic_cast<U*>(ptr))
				return Ref<U>(m_id);
		}
		return Ref<U>();
	}

	bool operator==(const Ref &other) const
	{
		return m_id == other.m_id;
	}

	bool operator!=(const Ref &other) const
	{
		return m_id != other.m_id;
	}

	bool operator==(const T *ptr) const
	{
		return get() == ptr;
	}

	bool operator!=(const T *ptr) const
	{
		return get() != ptr;
	}

	bool operator==(T *ptr) const
	{
		return get() == ptr;
	}

	bool operator!=(T *ptr) const
	{
		return get() != ptr;
	}

private:
	ObjectId m_id;
};

template <typename T>
inline bool operator==(const T *ptr, const Ref<T> &ref) { return ref.get() == ptr; }
template <typename T>
inline bool operator!=(const T *ptr, const Ref<T> &ref) { return ref.get() != ptr; }
template <typename T>
inline bool operator==(T *ptr, const Ref<T> &ref) { return ref.get() == ptr; }
template <typename T>
inline bool operator!=(T *ptr, const Ref<T> &ref) { return ref.get() != ptr; }

#endif // REF_H
