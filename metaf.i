%module metaf

%{
#include "metaf_wrapper.hpp"
%}

// Handle exceptions
%include "exception.i"
%exception {
    try {
        $action
    } catch (const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (...) {
        SWIG_exception(SWIG_UnknownError, "Unknown exception");
    }
}

// Handle std::string
%include "std_string.i"

// Handle std::vector
%include "std_vector.i"

// Template instantiations for the types we use
%template(StringVector) std::vector<std::string>;

// Include our simplified wrapper
%include "metaf_wrapper.hpp"
