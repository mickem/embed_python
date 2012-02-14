#include "python_callins.hpp"

#include <boost/python.hpp>

namespace bp = boost::python;


void hello(int id) {
	std::cout << "hello_cpp(" << id << ")\n";
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

void call_python(bp::dict &localDict, int id) {
	try	{
		bp::object scriptFunction = bp::extract<bp::object>(localDict["hello_python"]);
		if(scriptFunction)
			scriptFunction(id);
		else
			std::cout << "Script did not have a hello function!\n";
	} catch(const bp::error_already_set &e) {
		std::cout << "Exception in script: ";
		print_py_error();
	} catch(const std::exception &e) {
		std::cout << "Exception in script: " << e.what() << "\n";
	} catch(...) {
		std::cout << "Exception in script: UNKNOWN\n";
	}
}

int MAIN(int argc, const unicode_char* argv[]) {
	Py_Initialize();

	initTEST();

	try	{
		bp::object main_module = bp::import("__main__");
		bp::dict globalDict = bp::extract<bp::dict>(main_module.attr("__dict__"));
		bp::dict localDict = globalDict.copy();

		bp::object ignored = bp::exec(
			"from TEST import hello_cpp\n"
			"\n"
			"def hello_python(id):\n"
			"	hello_cpp(id)\n"
			"\n"
			, localDict, localDict);

		call_python(localDict, 1234);
	} catch(const bp::error_already_set &e) {
		std::cout << "Exception in script: ";
		print_py_error();
	} catch(const std::exception &e) {
		std::cout << "Exception in script: " << e.what() << "\n";
	} catch(...) {
		std::cout << "Exception in script: UNKNOWN\n";
	}
}

