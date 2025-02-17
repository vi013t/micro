#ifndef CONTEXT_H
#define CONTEXT_H

#include "list.h"
#include "parser.h"

typedef struct Scope Scope;

DEFINE_KLEIN_LIST(Scope);

typedef struct {
	String name;
	Value value;
} ScopeDeclaration;

DEFINE_KLEIN_LIST(ScopeDeclaration);

struct Scope {
	Scope* parent;
	ScopeList children;
	ScopeDeclarationList variables;
};

typedef struct Context Context;
struct Context {
	Scope* scope;
	Scope globalScope;
	int debugIndent;
};

/**
 * Creates a new context allocated on the heap, wrapped in a
 * `Result`. The caller is responsible for freeing the memory, most
 * likely using `freeContext()`.
 *
 * # Returns
 *
 * The context pointer wrapped in a result.
 *
 * # Errors
 *
 * If memory fails to allocate, an error is returned.
 */
KleinResult newContext(Context* output);

/**
 * Declares a new variable in the given scope with the given name and
 * value.
 *
 * # Parameters
 *
 * - `scope` - The scope to add the variable to, which will be mutated to
 *   reflect the added variable.
 * - `name` - The name of the variable as a null-terminated string that lives
 *   at least as long as the given scope object.
 * - `value` - The value of the variable.
 *
 * # Returns
 *
 * `NULL` wrapped in a `Result`, assuming success.
 *
 * # Errors
 *
 * If memory fails to allocate for the new variable, an error is returned.
 * If a variable already exists in the given scope (or any of its parent
 * scopes), an error is returned. If the given `scope` or `name` is `NULL`,
 * an error is returned.
 */
KleinResult declareNewVariable(Scope* scope, ScopeDeclaration declaration);

/**
 * Returns a pointer to the `Expression` value stored in the variable with the
 * given name in the given scope. This will traverse up the scope hierarchy
 * to the global scope until the variable is found.
 *
 * # Parameters
 *
 * - `scope` - The scope to search in.
 * - `name` - The name of the variable as a null-terminated string. It only
 *   needs to be valid for the duration of the function call; It can be safely
 *   freed as soon as this function is finished. Doing so will not invalidate
 *   the returned `Expression` pointer.
 *
 * # Returns
 *
 * The variable's `Expression` value, wrapped in a `Result`, assuming success.
 *
 * # Errors
 *
 * If the given `name` is `NULL`, an error is returned.
 * If no variable exists with the given name in the given scope,
 * an error is returned.
 */
KleinResult getVariable(Scope scope, char* name, Value** output);
KleinResult setVariable(Scope* scope, ScopeDeclaration declaration);
KleinResult reassignVariable(Scope* scope, ScopeDeclaration declaration);

KleinResult enterNewScope(void);
KleinResult exitScope(void);

void freeContext(Context context);

#endif
