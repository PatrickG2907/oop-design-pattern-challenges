import threading

class Logger:
    _instance = None
    _instance_lock = threading.Lock()

    def __new__(cls):
        # Double-checked locking
        if cls._instance is None:
            with cls._instance_lock:
                if cls._instance is None:
                    cls._instance = super().__new__(cls)
                    cls._instance._logs = []
                    cls._instance._log_lock = threading.Lock()
        return cls._instance

    def log(self, message: str):
        with self._log_lock:
            self._logs.append(message)

    def show_logs(self):
        with self._log_lock:
            for msg in self._logs:
                print(msg)

# ----------- Demonstration (multi-threaded) -----------

def worker(thread_id):
    logger = Logger()
    logger.log(f"Log from thread {thread_id} | logger id = {id(logger)}")

if __name__ == "__main__":
    threads = []

    for i in range(5):
        t = threading.Thread(target=worker, args=(i,))
        threads.append(t)
        t.start()

    for t in threads:
        t.join()

    # Verify singleton behavior
    logger1 = Logger()
    logger2 = Logger()

    print("\nSame instance check:")
    print(id(logger1), id(logger2))

    print("\nCollected logs:")
    logger1.show_logs()
