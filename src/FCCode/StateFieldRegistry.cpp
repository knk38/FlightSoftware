#include "StateField.hpp"

DataField::DataField(const std::string& name) : Nameable(name) {}

StateFieldRegistry::StateFieldRegistry() : 
                                    Debuggable(),
                                    _fields_allowed_to_read(),
                                    _fields_allowed_to_write() {}

void StateFieldRegistry::operator=(const StateFieldRegistry& r) {
    _fields_allowed_to_read = r._fields_allowed_to_read;
    _fields_allowed_to_write = r._fields_allowed_to_write;
}

void StateFieldRegistry::add_reader(Task& reader, DataField& field) {
    // Add field to registry if it doesn't exist
    if (_fields.find(field.name()) == _fields.end()) _fields.insert({field.name(), &field});

    if (_fields_allowed_to_read.count(&reader) == 0) {
        // TODO add debug console
        _fields_allowed_to_read.emplace(&reader, *(new std::vector<DataField*>) );
    }
    // TODO
    _fields_allowed_to_read.at(&reader).push_back(&field);
}

void StateFieldRegistry::add_writer(Task& writer, DataField& field) {
    // Add field to registry if it doesn't exist
    if (_fields.find(field.name()) == _fields.end()) _fields.insert({field.name(), &field});

    if (_fields_allowed_to_write.count(&writer) == 0) {
        // TODO add debug console
        _fields_allowed_to_write.emplace(&writer, *(new std::vector<DataField*>) );
    }
    // TODO
    _fields_allowed_to_write.at(&writer).push_back(&field);
}

bool StateFieldRegistry::can_read(Task& reader, DataField& field) {
    // TODO
    return false;
}

bool StateFieldRegistry::can_write(Task& reader, DataField& field) {
    // TODO
    return false;
}