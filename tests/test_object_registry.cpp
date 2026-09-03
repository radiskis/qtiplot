#include "ObjectId.h"
#include "ObjectRegistry.h"
#include "Ref.h"
#include <QCoreApplication>
#include <cassert>
#include <iostream>

class BaseWidget : public QObject
{
	Q_OBJECT
public:
	explicit BaseWidget(QObject *parent = nullptr) : QObject(parent) {}
};

class TableWidget : public BaseWidget
{
	Q_OBJECT
public:
	explicit TableWidget(QObject *parent = nullptr) : BaseWidget(parent) {}
};

class GraphWidget : public BaseWidget
{
	Q_OBJECT
public:
	explicit GraphWidget(QObject *parent = nullptr) : BaseWidget(parent) {}
};

int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);

	ObjectRegistry *registry = ObjectRegistry::instance();
	assert(registry != nullptr);

	std::cout << "[TEST] 1. Registering objects and verifying unique ObjectIds..." << std::endl;
	TableWidget *table = new TableWidget();
	GraphWidget *graph = new GraphWidget();

	ObjectId tableId = registry->registerObject(table, "TableWidget");
	ObjectId graphId = registry->registerObject(graph, "GraphWidget");

	assert(tableId != NullObjectId);
	assert(graphId != NullObjectId);
	assert(tableId != graphId);
	assert(registry->idOf(table) == tableId);
	assert(registry->idOf(graph) == graphId);
	assert(registry->isAlive(tableId));
	assert(registry->isAlive(graphId));
	assert(registry->typeOf(tableId) == "TableWidget");

	std::cout << "[TEST] 2. Ref<T> smart pointer dereferencing and resolution..." << std::endl;
	Ref<TableWidget> refTable(table);
	Ref<GraphWidget> refGraph(graph);

	assert(refTable.id() == tableId);
	assert(refTable.get() == table);
	assert(refTable.isValid());
	assert(static_cast<bool>(refTable) == true);
	assert(refTable == table);

	std::cout << "[TEST] 3. Ref<T>::as<U>() safe downcasting..." << std::endl;
	Ref<BaseWidget> refBase(table);
	assert(refBase.id() == tableId);
	assert(refBase.get() == table);

	Ref<TableWidget> refDowncast = refBase.as<TableWidget>();
	assert(refDowncast.isValid());
	assert(refDowncast.get() == table);

	Ref<GraphWidget> refBadDowncast = refBase.as<GraphWidget>();
	assert(!refBadDowncast.isValid());
	assert(refBadDowncast.get() == nullptr);

	std::cout << "[TEST] 4. Reverse-dependency index..." << std::endl;
	// Graph depends on Table (e.g. plot curve referencing table column)
	registry->addDependency(graphId, tableId);
	QSet<ObjectId> tableDependents = registry->dependentsOf(tableId);
	assert(tableDependents.contains(graphId));
	QSet<ObjectId> graphDeps = registry->dependenciesOf(graphId);
	assert(graphDeps.contains(tableId));

	std::cout << "[TEST] 5. Object destruction and stale Ref invalidation..." << std::endl;
	registry->unregisterObject(table);
	delete table;
	table = nullptr;

	assert(!registry->isAlive(tableId));
	assert(registry->resolve(tableId) == nullptr);
	// Stale refTable must resolve to nullptr immediately without dangling pointers
	assert(refTable.get() == nullptr);
	assert(!refTable.isValid());
	assert(!refTable);

	// Reverse dependencies must be automatically purged
	assert(registry->dependentsOf(tableId).isEmpty());
	assert(registry->dependenciesOf(graphId).isEmpty());

	std::cout << "[TEST] 6. Cleanup remaining objects..." << std::endl;
	registry->unregisterObject(graph);
	delete graph;
	graph = nullptr;

	assert(refGraph.get() == nullptr);
	assert(!refGraph.isValid());

	std::cout << "[PASS] All ObjectRegistry and Ref<T> tests passed successfully!" << std::endl;
	return 0;
}

#include "test_object_registry.moc"
