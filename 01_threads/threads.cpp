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

void thread_proc(const int id, const int delay) {
	boost::posix_time::seconds time_to_sleep(delay);
	safe_cout << "ThreadProc[" << id << "]: started\n";

	boost::this_thread::sleep(time_to_sleep);
	safe_cout << "ThreadProc[" << id << "]: finished\n";
}  

int MAIN(int argc, const unicode_char* argv[]) {
	boost::thread_group threads;

	for (int i=0;i<10;i++) {
		threads.create_thread(boost::bind(&thread_proc, i, 5));
	}

	safe_cout << "main: waiting for threads to join\n";
	threads.join_all();  
}

