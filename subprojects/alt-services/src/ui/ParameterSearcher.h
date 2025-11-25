#pragma once

#include "ItemModelSearcher.h"
#include "data/models/ParameterModel.h"

/*
 * Base class for searching inside a ParameterModel
 */
class ParameterSearcher : public ItemModelSearcher {
    Q_OBJECT

public:
    inline ParameterSearcher(ParameterModel* model = nullptr)
        : ItemModelSearcher { model }
    {}

    inline void setModel(ParameterModel& model){ ItemModelSearcher::setModel(model); }
    inline ParameterModel* model() { return static_cast<ParameterModel*>(ItemModelSearcher::model()); }

    inline Parameter::Value* prevValue()
    { return model()->indexToValue(ItemModelSearcher::prevIndex()); }

    inline Parameter::Value* nextValue()
    { return model()->indexToValue(ItemModelSearcher::nextIndex()); }
};
