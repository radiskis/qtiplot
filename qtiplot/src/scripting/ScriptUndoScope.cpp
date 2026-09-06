#include "ScriptUndoScope.h"
#include <QUndoStack>

QList<ScriptUndoScope*> ScriptUndoScope::s_scope_stack;
bool ScriptUndoScope::s_batch_runner_active = false;

ScriptUndoScope::ScriptUndoScope(const QString &macroName)
	: d_macro_name(macroName.isEmpty() ? QStringLiteral("Script Execution") : macroName)
{
	s_scope_stack.append(this);
}

ScriptUndoScope::~ScriptUndoScope()
{
	for (const QPointer<QUndoStack> &stack : d_opened_stacks) {
		if (stack)
			stack->endMacro();
	}
	d_opened_stacks.clear();

	if (!s_scope_stack.isEmpty() && s_scope_stack.last() == this)
		s_scope_stack.removeLast();
	else
		s_scope_stack.removeAll(this);
}

bool ScriptUndoScope::isActive()
{
	return !s_scope_stack.isEmpty();
}

bool ScriptUndoScope::isBatchRunnerActive()
{
	return s_batch_runner_active;
}

void ScriptUndoScope::setBatchRunnerActive(bool active)
{
	s_batch_runner_active = active;
}

QString ScriptUndoScope::currentMacroName()
{
	if (s_scope_stack.isEmpty())
		return QStringLiteral("Script Execution");
	return s_scope_stack.last()->d_macro_name;
}

void ScriptUndoScope::registerStack(QUndoStack *stack)
{
	if (!stack || s_scope_stack.isEmpty())
		return;

	ScriptUndoScope *current = s_scope_stack.last();
	for (const QPointer<QUndoStack> &s : current->d_opened_stacks) {
		if (s == stack)
			return;
	}
	current->d_opened_stacks.append(stack);
	stack->beginMacro(current->d_macro_name);
}
