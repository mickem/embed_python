#include "threads.hpp"

#include <boost/thread.hpp>


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

void thread_proc(const int id, const int delay) {
	for (int i=0;i<thread_loops;i++) {
		boost::posix_time::millisec time_to_sleep(rand()*delay/RAND_MAX);
		std::stringstream ss;
		ss << ">>> proc: " << id << "\n";
		safe_cout << ss.str();
		boost::this_thread::sleep(time_to_sleep);
	}
}  

int MAIN(int argc, const unicode_char* argv[]) {
	boost::thread_group threads;

	for (int i=0;i<thread_count;i++) {
		threads.create_thread(boost::bind(&thread_proc, i, 5));
	}

	safe_cout << "main: waiting for threads to join\n";
	threads.join_all();  
}

