#ifndef REGISTERED_H
#define REGISTERED_H

#include "ObjectId.h"
#include "ObjectRegistry.h"
#include <typeinfo>
#include <cstddef>

class RegisteredBase {
public:
    ObjectId objectId() const { return d_id; }

protected:
    RegisteredBase(void *ptr, const char *typeName) {
        d_id = ObjectRegistry::instance()->registerObject(ptr, QString::fromUtf8(typeName));
    }

    virtual ~RegisteredBase() {
        if (d_id != NullObjectId) {
            ObjectRegistry::instance()->unregisterId(d_id);
            d_id = NullObjectId;
        }
    }

private:
    ObjectId d_id = NullObjectId;
};

template <class Derived>
class Registered : public RegisteredBase {
protected:
#if defined(__clang__)
    __attribute__((no_sanitize("undefined", "vptr")))
#elif defined(__GNUC__)
    __attribute__((no_sanitize_undefined))
#endif
    Registered() : RegisteredBase(static_cast<Derived*>(this), typeid(Derived).name()) {}
};

#endif // REGISTERED_H