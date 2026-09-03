#include "ObjectId.h"
#include "ObjectRegistry.h"
#include "Registered.h"
#include "Ref.h"
#include <QCoreApplication>
#include <iostream>

#define TEST_CHECK(cond) \
	do { \
		if (!(cond)) { \
			std::cerr << "CHECK FAILED: " #cond " at " __FILE__ ":" << __LINE__ << std::endl; \
			return 1; \
		} \
	} while (0)

class BaseWidget : public QObject, public Registered<BaseWidget>
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
	TEST_CHECK(registry != nullptr);

	std::cout << "[TEST] 1. Registering objects and verifying unique ObjectIds..." << std::endl;
	TableWidget *table = new TableWidget();
	GraphWidget *graph = new GraphWidget();

	ObjectId tableId = table->objectId();
	ObjectId graphId = graph->objectId();

	TEST_CHECK(tableId != NullObjectId);
	TEST_CHECK(graphId != NullObjectId);
	TEST_CHECK(tableId != graphId);
	TEST_CHECK(registry->idOf(table) == tableId);
	TEST_CHECK(registry->idOf(graph) == graphId);
	TEST_CHECK(registry->isAlive(tableId));
	TEST_CHECK(registry->isAlive(graphId));

	std::cout << "[TEST] 2. Ref<T> smart pointer dereferencing and resolution..." << std::endl;
	Ref<TableWidget> refTable(table);
	Ref<GraphWidget> refGraph(graph);

	TEST_CHECK(refTable.id() == tableId);
	TEST_CHECK(refTable.get() == table);
	TEST_CHECK(refTable.isValid());
	TEST_CHECK(static_cast<bool>(refTable) == true);
	TEST_CHECK(refTable == table);

	std::cout << "[TEST] 3. Ref<T>::as<U>() safe downcasting..." << std::endl;
	Ref<BaseWidget> refBase(table);
	TEST_CHECK(refBase.id() == tableId);
	TEST_CHECK(refBase.get() == table);

	Ref<TableWidget> refDowncast = refBase.as<TableWidget>();
	TEST_CHECK(refDowncast.isValid());
	TEST_CHECK(refDowncast.get() == table);

	Ref<GraphWidget> refBadDowncast = refBase.as<GraphWidget>();
	TEST_CHECK(!refBadDowncast.isValid());
	TEST_CHECK(refBadDowncast.get() == nullptr);

	std::cout << "[TEST] 4. Reverse-dependency index..." << std::endl;
	// Graph depends on Table (e.g. plot curve referencing table column)
	registry->addDependency(graphId, tableId);
	QSet<ObjectId> tableDependents = registry->dependentsOf(tableId);
	TEST_CHECK(tableDependents.contains(graphId));
	QSet<ObjectId> graphDeps = registry->dependenciesOf(graphId);
	TEST_CHECK(graphDeps.contains(tableId));

	std::cout << "[TEST] 5. Object destruction and automated stale Ref invalidation..." << std::endl;
	// Destructor of Registered<BaseWidget> must automatically unregister table!
	delete table;
	table = nullptr;

	TEST_CHECK(!registry->isAlive(tableId));
	TEST_CHECK(registry->resolve(tableId) == nullptr);
	// Stale refTable must resolve to nullptr immediately without dangling pointers
	TEST_CHECK(refTable.get() == nullptr);
	TEST_CHECK(!refTable.isValid());
	TEST_CHECK(!refTable);

	// Reverse dependencies must be automatically purged
	TEST_CHECK(registry->dependentsOf(tableId).isEmpty());
	TEST_CHECK(registry->dependenciesOf(graphId).isEmpty());

	std::cout << "[TEST] 6. Cleanup remaining objects automatically via destructor..." << std::endl;
	delete graph;
	graph = nullptr;

	TEST_CHECK(!registry->isAlive(graphId));
	TEST_CHECK(refGraph.get() == nullptr);
	TEST_CHECK(!refGraph.isValid());

	std::cout << "[PASS] All ObjectRegistry and Ref<T> tests passed successfully!" << std::endl;
	return 0;
}

#include "test_object_registry.moc"
