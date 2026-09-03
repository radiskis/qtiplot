#ifndef OBJECT_REGISTRY_H
#define OBJECT_REGISTRY_H

#include "ObjectId.h"
#include <QObject>
#include <QHash>
#include <QSet>
#include <QString>
#include <atomic>

class ObjectRegistry : public QObject
{
	Q_OBJECT

public:
	static ObjectRegistry* instance();

	explicit ObjectRegistry(QObject *parent = nullptr);
	~ObjectRegistry() override;

	ObjectId registerObject(void *ptr, const QString &typeName = QString());
	void unregisterObject(void *ptr);
	void unregisterId(ObjectId id);

	void* resolve(ObjectId id) const;

	template <typename T>
	T* resolve(ObjectId id) const
	{
		void *ptr = resolve(id);
		if (!ptr)
			return nullptr;
		if constexpr (std::is_base_of_v<QObject, T>) {
			return dynamic_cast<T*>(static_cast<QObject*>(ptr));
		} else {
			return static_cast<T*>(ptr);
		}
	}

	ObjectId idOf(const void *ptr) const;
	QString typeOf(ObjectId id) const;
	bool isAlive(ObjectId id) const;

	void addDependency(ObjectId dependent, ObjectId dependency);
	void removeDependency(ObjectId dependent, ObjectId dependency);
	void clearDependenciesFor(ObjectId id);

	QSet<ObjectId> dependentsOf(ObjectId dependency) const;
	QSet<ObjectId> dependenciesOf(ObjectId dependent) const;

	void clear();

signals:
	void objectRegistered(ObjectId id, void *ptr);
	void objectUnregistered(ObjectId id);
	void dependencyAdded(ObjectId dependent, ObjectId dependency);
	void dependencyRemoved(ObjectId dependent, ObjectId dependency);

private:
	std::atomic<uint64_t> m_nextId{1};

	QHash<ObjectId, void*> m_idToPtr;
	QHash<void*, ObjectId> m_ptrToId;
	QHash<ObjectId, QString> m_idToType;

	// Reverse-dependency index: dependency -> set of dependents
	QHash<ObjectId, QSet<ObjectId>> m_dependents;
	// Forward-dependency index: dependent -> set of dependencies
	QHash<ObjectId, QSet<ObjectId>> m_dependencies;
};

#endif // OBJECT_REGISTRY_H
