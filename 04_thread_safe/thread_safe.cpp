#include "thread_safe.hpp"

#include <boost/python.hpp>
#include <boost/thread.hpp>

namespace bp = boost::python;

class logger {
	boost::recursive_mutex cout_guard;
public:
	template <typename T>
	logger & operator << (const T & data){
		boost::lock_guard<boost::recursive_mutex> lock(cout_guard);
		std::cout << data;
		return *this;
	}
};
logger safe_cout;

static int delay = 100;
static int thread_count = 100;
static int thread_loops = 10;

struct aquire_py_GIL {
	PyGILState_STATE state;
	aquire_py_GIL() {
		state = PyGILState_Ensure();
	}

	~aquire_py_GIL() {
		PyGILState_Release(state);
	}
};
struct release_py_GIL {
	PyThreadState *state;
	release_py_GIL() {
		state = PyEval_SaveThread();
	}
	~release_py_GIL() {
		PyEval_RestoreThread(state);
	}
};

void hello(int id) {
	release_py_GIL unlocker;
	std::stringstream ss;
	ss << ">>> py: sleep: " << id << "\n";
	safe_cout << ss.str();
	boost::this_thread::sleep(boost::posix_time::millisec(rand()*delay/RAND_MAX));
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
		safe_cout << err_text << "\n";
	} catch (...) {
		safe_cout << "Failed to parse python error\n";
	}
	PyErr_Clear();
}


void call_python(bp::dict &localDict, int id) {
	try {
		aquire_py_GIL lock;
		try	{
			bp::object scriptFunction = bp::extract<bp::object>(localDict["hello_python"]);
			if(scriptFunction)
				scriptFunction(id);
			else
				safe_cout << "Script did not have a hello function!\n";
		} catch(const bp::error_already_set &e) {
			safe_cout << "Exception in script: ";
			print_py_error();
		}
	} catch(const std::exception &e) {
		safe_cout << "Exception in script: " << e.what() << "\n";
	}
}

void thread_proc(const int id, bp::dict localDict) {
	for (int i=0;i<thread_loops;i++) {
		boost::posix_time::millisec time_to_sleep(rand()*delay/RAND_MAX);
		std::stringstream ss;
		ss << ">>> proc: " << id << "\n";
		safe_cout << ss.str();
		boost::this_thread::sleep(time_to_sleep);
		call_python(localDict, id);
	}
}  


int MAIN(int argc, const unicode_char* argv[]) {
	Py_Initialize();
	PyEval_InitThreads();
	initTEST();

	try	{
		bp::object main_module = bp::import("__main__");
		bp::dict globalDict = bp::extract<bp::dict>(main_module.attr("__dict__"));
		bp::dict localDict = globalDict.copy();

		try {
			bp::object ignored = bp::exec(
				"from TEST import hello_cpp\n"
				"\n"
				"def hello_python(id):\n"
				"	hello_cpp(id)\n"
				"\n"
				, localDict, localDict);
		} catch(const bp::error_already_set &e) {
			safe_cout << "Exception in script: ";
			print_py_error();
		}

		PyThreadState *state = PyEval_SaveThread();

		boost::thread_group threads;
		for (int i=0;i<thread_count;i++)
			threads.create_thread(boost::bind(&thread_proc, i, localDict));
		safe_cout << ":::main: waiting for threads to join\n";
		threads.join_all();  

	} catch(const std::exception &e) {
		safe_cout << "Exception in script: " << e.what() << "\n";
	}
}

