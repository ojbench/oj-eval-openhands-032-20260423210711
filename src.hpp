#ifndef PYLIST_H
#define PYLIST_H

#include <iostream>
#include <memory>
#include <vector>
#include <any>

class pylist {
private:
    // Element can be either an int or a nested pylist
    // Using std::any to avoid circular dependency
    struct Element {
        std::any value;
        bool is_int_flag;
        
        Element() : value(0), is_int_flag(true) {}
        Element(int i) : value(i), is_int_flag(true) {}
        Element(const pylist& pl) : value(pl), is_int_flag(false) {}
        
        // Assignment operators
        Element& operator=(int i) {
            value = i;
            is_int_flag = true;
            return *this;
        }
        
        Element& operator=(const pylist& pl) {
            value = pl;
            is_int_flag = false;
            return *this;
        }
        
        // Access methods
        pylist& as_pylist() {
            return std::any_cast<pylist&>(value);
        }
        
        const pylist& as_pylist() const {
            return std::any_cast<const pylist&>(value);
        }
        
        bool is_int() const {
            return is_int_flag;
        }
        
        int as_int() const {
            return std::any_cast<int>(value);
        }
    };
    
    // Shared data structure to enable reference semantics
    std::shared_ptr<std::vector<Element>> data;
    bool is_scalar; // Flag to indicate if this represents a single int value

public:
    pylist() : data(std::make_shared<std::vector<Element>>()), is_scalar(false) {}
    
    // Special constructor for scalar values
    pylist(int val, bool scalar) : data(std::make_shared<std::vector<Element>>()), is_scalar(scalar) {
        if (scalar) {
            data->push_back(Element(val));
        }
    }
    
    // Copy constructor - shares the data (reference semantics)
    pylist(const pylist& other) : data(other.data), is_scalar(other.is_scalar) {}
    
    // Assignment operator - shares the data (reference semantics)
    pylist& operator=(const pylist& other) {
        data = other.data;
        is_scalar = other.is_scalar;
        return *this;
    }
    
    void append(int x) {
        data->push_back(Element(x));
    }
    
    void append(const pylist& x) {
        data->push_back(Element(x));
    }
    
    pylist pop() {
        if (data->empty()) {
            return pylist();
        }
        Element elem = data->back();
        data->pop_back();
        if (elem.is_int()) {
            return pylist(elem.as_int(), true); // Return as scalar
        } else {
            return elem.as_pylist();
        }
    }
    
    // Conversion operator to int for scalar values
    operator int() const {
        if (is_scalar && !data->empty()) {
            return (*data)[0].as_int();
        }
        return 0;
    }
    
    // Proxy class to handle both reading and writing
    class ElementProxy {
    private:
        Element& elem;
        
    public:
        ElementProxy(Element& e) : elem(e) {}
        
        // Allow reading as int
        operator int() const {
            if (elem.is_int()) {
                return elem.as_int();
            }
            return 0;
        }
        
        // Assignment from int
        ElementProxy& operator=(int i) {
            elem = i;
            return *this;
        }
        
        // Assignment from pylist
        ElementProxy& operator=(const pylist& pl) {
            elem = pl;
            return *this;
        }
        
        // Assignment from another proxy
        ElementProxy& operator=(const ElementProxy& other) {
            if (other.elem.is_int()) {
                elem = other.elem.as_int();
            } else {
                elem = other.elem.as_pylist();
            }
            return *this;
        }
        
        // Access nested list element
        ElementProxy operator[](size_t i) {
            return elem.as_pylist()[i];
        }
        
        // Support append for nested lists
        void append(int x) {
            elem.as_pylist().append(x);
        }
        
        void append(const pylist& x) {
            elem.as_pylist().append(x);
        }
        
        // Support pop for nested lists
        pylist pop() {
            return elem.as_pylist().pop();
        }
        
        // Support taking address (returns pointer to underlying pylist)
        pylist* operator&() {
            if (!elem.is_int()) {
                return &(elem.as_pylist());
            }
            return nullptr;
        }
        
        // Output operator
        friend std::ostream& operator<<(std::ostream& os, const ElementProxy& proxy) {
            if (proxy.elem.is_int()) {
                os << proxy.elem.as_int();
            } else {
                os << proxy.elem.as_pylist();
            }
            return os;
        }
    };
    
    ElementProxy operator[](size_t i) {
        return ElementProxy((*data)[i]);
    }
    
    friend std::ostream& operator<<(std::ostream& os, const pylist& ls) {
        if (ls.is_scalar && !ls.data->empty()) {
            // Print as scalar int
            os << (*ls.data)[0].as_int();
        } else {
            // Print as list
            os << "[";
            for (size_t i = 0; i < ls.data->size(); ++i) {
                if (i > 0) os << ", ";
                const Element& elem = (*ls.data)[i];
                if (elem.is_int()) {
                    os << elem.as_int();
                } else {
                    os << elem.as_pylist();
                }
            }
            os << "]";
        }
        return os;
    }
};

#endif //PYLIST_H
