#ifndef REGISTERED_H
#define REGISTERED_H

#include "ObjectId.h"
#include "ObjectRegistry.h"
#include <typeinfo>

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
    Registered() : RegisteredBase(static_cast<Derived*>(this), typeid(Derived).name()) {}
};

#endif // REGISTERED_H