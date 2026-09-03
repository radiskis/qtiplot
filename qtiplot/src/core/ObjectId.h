#ifndef OBJECT_ID_H
#define OBJECT_ID_H

#include <cstdint>

/**
 * @brief Strongly typed 64-bit identifier for project entities.
 *
 * An ObjectId is a non-zero, never-recycled integer assigned to objects
 * (tables, matrices, plots, curves, layers, etc.) when registered with
 * the ObjectRegistry.
 */
using ObjectId = uint64_t;

constexpr ObjectId NullObjectId = 0;

#endif // OBJECT_ID_H
