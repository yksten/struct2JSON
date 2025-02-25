#ifndef __JSON_DECODER_H__
#define __JSON_DECODER_H__

#include <map>
#include <serialflex/traits.h>

namespace serialflex {

namespace custom {
class Reader;
struct GenericValue;
}// namespace custom

class EXPORTAPI JSONDecoder {
    bool convert_by_type_; // convert by field type
    bool case_insensitive_;// key case insensitive
    custom::Reader* reader_;
    const custom::GenericValue* current_;

    JSONDecoder(const JSONDecoder&);
    JSONDecoder& operator=(const JSONDecoder&);

public:
    JSONDecoder(const char* str, bool caseInsensitive = false);
    ~JSONDecoder();

    // convert by field type
    JSONDecoder& setConvertByType(bool convert_by_type);

    template <typename T>
    JSONDecoder& convert(const char* name, T& value, bool* has_value = NULL) {
        decodeValue(name, *(typename internal::TypeTraits<T>::Type*)(&value),
                    has_value);
        return *this;
    }

    template <typename T> bool operator>>(T& value) {
        if (!current_) {
            return false;
        }
        const custom::GenericValue* parent = current_;
        internal::serializeWrapper(*this, *const_cast<T*>(&value));
        return (parent == current_);
    }

    template <typename T> bool operator>>(std::vector<T>& value) {
        if (!current_) {
            return false;
        }
        const custom::GenericValue* parent = current_;
        uint32_t size = JSONDecoder::getObjectSize(current_);
        if (size) {
            value.resize(size);
        }
        const custom::GenericValue* parent_temp = current_;
        current_ = JSONDecoder::getChild(current_);
        for (uint32_t idx = 0; current_ && (idx < size);
             (current_ = JSONDecoder::getNext(current_)) && ++idx) {
            decodeValue(
                NULL,
                *(typename internal::TypeTraits<T>::Type*)(&value.at(idx)),
                NULL);
        }
        current_ = parent_temp;
        return (parent == current_);
    }

    template <typename K, typename V> bool operator>>(std::map<K, V>& value) {
        if (!current_) {
            return false;
        }
        const custom::GenericValue* parent = current_;
        value.clear();
        const custom::GenericValue* parent_temp = current_;
        for (const custom::GenericValue* child =
                 JSONDecoder::getChild(parent);
             child; child = JSONDecoder::getNext(child)) {
            std::string key(JSONDecoder::getKey(child),
                            JSONDecoder::getKeySize(child));
            V item = V();
            decodeValue(key.c_str(), item, NULL);
            value.insert(std::pair<K, V>(
                internal::STOT::type<K>::strto(key.c_str()), item));
        }
        current_ = parent_temp;
        return (parent == current_);
    }

private:
    template <typename T>
    void decodeValue(const char* name, T& value, bool* has_value) {
        const custom::GenericValue* parent = current_;
        current_ =
            JSONDecoder::getObjectItem(current_, name, case_insensitive_);
        if (current_) {
            internal::serializeWrapper(*this, value);
            if (has_value) *has_value = true;
        }
        current_ = parent;
    }

    template <typename T>
    void decodeValue(const char* name, std::vector<T>& value, bool* has_value) {
        const custom::GenericValue* parent = current_;
        current_ =
            JSONDecoder::getObjectItem(current_, name, case_insensitive_);
        if (current_) {
            uint32_t size = JSONDecoder::getObjectSize(current_);
            if (size) {
                value.resize(size);
            }
            const custom::GenericValue* parent_temp = current_;
            current_ = JSONDecoder::getChild(current_);
            for (uint32_t idx = 0; current_ && (idx < size);
                 (current_ = JSONDecoder::getNext(current_)) && ++idx) {
                decodeValue(
                    NULL,
                    *(typename internal::TypeTraits<T>::Type*)(&value.at(idx)),
                    NULL);
            }
            current_ = parent_temp;
            if (has_value) {
                *has_value = true;
            }
        }
        current_ = parent;
    }

    template <typename K, typename V>
    void decodeValue(const char* name, std::map<K, V>& value, bool* has_value) {
        const custom::GenericValue* parent = current_;
        current_ =
            JSONDecoder::getObjectItem(current_, name, case_insensitive_);
        if (current_) {
            value.clear();

            const custom::GenericValue* parent_temp = current_;
            for (current_ = JSONDecoder::getChild(current_); current_;
                 current_ = JSONDecoder::getNext(current_)) {
                std::string key(JSONDecoder::getKey(current_),
                                JSONDecoder::getKeySize(current_));
                V item = V();
                decodeValue(NULL, item, NULL);
                value.insert(std::pair<K, V>(
                    internal::STOT::type<K>::strto(key.c_str()), item));
            }
            current_ = parent_temp;
            if (has_value) {
                *has_value = true;
            }
        }
        current_ = parent;
    }

    void decodeValue(const char* name, bool& value, bool* has_value);
    void decodeValue(const char* name, uint32_t& value, bool* has_value);
    void decodeValue(const char* name, int32_t& value, bool* has_value);
    void decodeValue(const char* name, uint64_t& value, bool* has_value);
    void decodeValue(const char* name, int64_t& value, bool* has_value);
    void decodeValue(const char* name, float& value, bool* has_value);
    void decodeValue(const char* name, double& value, bool* has_value);
    void decodeValue(const char* name, std::string& value, bool* has_value);
    void decodeValue(const char* name, std::vector<bool>& value,
                     bool* has_value);
    bool checkItemType(const custom::GenericValue& item, const int type) const;
    bool item2Bool(const custom::GenericValue& item) const;

    // for value
    static uint32_t getObjectSize(const custom::GenericValue* parent);
    static const custom::GenericValue*
    getObjectItem(const custom::GenericValue* parent, const char* name,
                  bool caseInsensitive);
    static const custom::GenericValue*
    getChild(const custom::GenericValue* parent);
    static const custom::GenericValue*
    getNext(const custom::GenericValue* parent);
    static const char* getKey(const custom::GenericValue* parent);
    static uint32_t getKeySize(const custom::GenericValue* parent);
};

}// namespace serialflex

#endif
