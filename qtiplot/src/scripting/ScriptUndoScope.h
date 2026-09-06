#ifndef SCRIPT_UNDO_SCOPE_H
#define SCRIPT_UNDO_SCOPE_H

#include <QString>
#include <QList>
#include <QPointer>

class QUndoStack;

//! RAII scope that groups all edits performed during a script execution into a single undo macro.
class ScriptUndoScope
{
public:
	explicit ScriptUndoScope(const QString &macroName = QString());
	~ScriptUndoScope();

	static bool isActive();
	static bool isBatchRunnerActive();
	static void setBatchRunnerActive(bool active);
	static void registerStack(QUndoStack *stack);
	static QString currentMacroName();

private:
	QString d_macro_name;
	QList<QPointer<QUndoStack>> d_opened_stacks;
	static QList<ScriptUndoScope*> s_scope_stack;
	static bool s_batch_runner_active;
};

#endif // SCRIPT_UNDO_SCOPE_H
