#include "ObjectRegistry.h"

static ObjectRegistry *s_instance = nullptr;

ObjectRegistry* ObjectRegistry::instance()
{
	if (!s_instance)
		s_instance = new ObjectRegistry();
	return s_instance;
}

ObjectRegistry::ObjectRegistry(QObject *parent)
	: QObject(parent)
{
	if (!s_instance)
		s_instance = this;
}

ObjectRegistry::~ObjectRegistry()
{
	if (s_instance == this)
		s_instance = nullptr;
}

ObjectId ObjectRegistry::registerObject(void *ptr, const QString &typeName)
{
	if (!ptr)
		return NullObjectId;

	auto it = m_ptrToId.constFind(ptr);
	if (it != m_ptrToId.constEnd())
		return it.value();

	ObjectId id = m_nextId.fetch_add(1, std::memory_order_relaxed);
	m_idToPtr.insert(id, ptr);
	m_ptrToId.insert(ptr, id);
	if (!typeName.isEmpty())
		m_idToType.insert(id, typeName);

	emit objectRegistered(id, ptr);
	return id;
}

void ObjectRegistry::unregisterObject(void *ptr)
{
	if (!ptr)
		return;

	auto it = m_ptrToId.find(ptr);
	if (it == m_ptrToId.end())
		return;

	ObjectId id = it.value();
	m_ptrToId.erase(it);
	m_idToPtr.remove(id);
	m_idToType.remove(id);

	clearDependenciesFor(id);

	emit objectUnregistered(id);
}

void ObjectRegistry::unregisterId(ObjectId id)
{
	if (id == NullObjectId)
		return;

	auto it = m_idToPtr.find(id);
	if (it == m_idToPtr.end())
		return;

	void *ptr = it.value();
	m_idToPtr.erase(it);
	m_ptrToId.remove(ptr);
	m_idToType.remove(id);

	clearDependenciesFor(id);

	emit objectUnregistered(id);
}

void* ObjectRegistry::resolve(ObjectId id) const
{
	if (id == NullObjectId)
		return nullptr;
	return m_idToPtr.value(id, nullptr);
}

ObjectId ObjectRegistry::idOf(const void *ptr) const
{
	if (!ptr)
		return NullObjectId;
	return m_ptrToId.value(const_cast<void*>(ptr), NullObjectId);
}

QString ObjectRegistry::typeOf(ObjectId id) const
{
	return m_idToType.value(id, QString());
}

bool ObjectRegistry::isAlive(ObjectId id) const
{
	if (id == NullObjectId)
		return false;
	return m_idToPtr.contains(id);
}

void ObjectRegistry::addDependency(ObjectId dependent, ObjectId dependency)
{
	if (dependent == NullObjectId || dependency == NullObjectId || dependent == dependency)
		return;

	m_dependents[dependency].insert(dependent);
	m_dependencies[dependent].insert(dependency);
	emit dependencyAdded(dependent, dependency);
}

void ObjectRegistry::removeDependency(ObjectId dependent, ObjectId dependency)
{
	if (dependent == NullObjectId || dependency == NullObjectId)
		return;

	auto itDep = m_dependents.find(dependency);
	if (itDep != m_dependents.end()) {
		itDep.value().remove(dependent);
		if (itDep.value().isEmpty())
			m_dependents.erase(itDep);
	}

	auto itFwd = m_dependencies.find(dependent);
	if (itFwd != m_dependencies.end()) {
		itFwd.value().remove(dependency);
		if (itFwd.value().isEmpty())
			m_dependencies.erase(itFwd);
	}

	emit dependencyRemoved(dependent, dependency);
}

void ObjectRegistry::clearDependenciesFor(ObjectId id)
{
	if (id == NullObjectId)
		return;

	// When an object is deleted or cleared:
	// 1. Notify and remove all dependencies where this object is the dependency (provider)
	auto itDep = m_dependents.find(id);
	if (itDep != m_dependents.end()) {
		QSet<ObjectId> dependents = itDep.value();
		m_dependents.erase(itDep);
		for (ObjectId d : dependents) {
			auto itFwd = m_dependencies.find(d);
			if (itFwd != m_dependencies.end()) {
				itFwd.value().remove(id);
				if (itFwd.value().isEmpty())
					m_dependencies.erase(itFwd);
			}
			emit dependencyRemoved(d, id);
		}
	}

	// 2. Remove all records where this object was a dependent
	auto itFwd = m_dependencies.find(id);
	if (itFwd != m_dependencies.end()) {
		QSet<ObjectId> dependencies = itFwd.value();
		m_dependencies.erase(itFwd);
		for (ObjectId provider : dependencies) {
			auto itD = m_dependents.find(provider);
			if (itD != m_dependents.end()) {
				itD.value().remove(id);
				if (itD.value().isEmpty())
					m_dependents.erase(itD);
			}
			emit dependencyRemoved(id, provider);
		}
	}
}

QSet<ObjectId> ObjectRegistry::dependentsOf(ObjectId dependency) const
{
	return m_dependents.value(dependency);
}

QSet<ObjectId> ObjectRegistry::dependenciesOf(ObjectId dependent) const
{
	return m_dependencies.value(dependent);
}

void ObjectRegistry::clear()
{
	m_idToPtr.clear();
	m_ptrToId.clear();
	m_idToType.clear();
	m_dependents.clear();
	m_dependencies.clear();
}
