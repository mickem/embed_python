#include "embed_python.hpp"

#include <boost/python.hpp>

namespace bp = boost::python;


void hello() {
	std::cout << "hello: from c++ (via Python)\n";
}

BOOST_PYTHON_MODULE(TEST)
{
	bp::def("hello_cpp", hello);
}

void print_py_error() {
	try {
		PyErr_Print();
		bp::object sys(bp::handle<>(PyImport_ImportModule("sys")));
		bp::object err = sys.attr("stderr");
		std::string err_text = bp::extract<std::string>(err.attr("getvalue")());
		std::cout << err_text << "\n";
	} catch (...) {
		std::cout << "Failed to parse python error\n";
	}
	PyErr_Clear();
}

int MAIN(int argc, const unicode_char* argv[]) {
	Py_Initialize();

	initTEST();

	try	{
		bp::object main_module = bp::import("__main__");
		bp::dict globalDict = bp::extract<bp::dict>(main_module.attr("__dict__"));
		bp::dict localDict = globalDict.copy();

		bp::object ignored = bp::exec(
			"from TEST import hello\n"
			"\n"
			"hello_cpp()\n"
			"\n"
			, localDict, localDict);	
	} catch(const bp::error_already_set &e) {
		std::cout << "Exception in script: ";
		print_py_error();
	} catch(const std::exception &e) {
		std::cout << "Exception in script: " << e.what() << "\n";
	} catch(...) {
		std::cout << "Exception in script: UNKNOWN\n";
	}
}

