/** xDDTools */
#pragma once

#include "xdd/proto.hpp"
#include "xdd/Scanner.hpp"
#include "xdd/file.hpp"
#include <QThread>
#include <QIcon>

namespace xdd {

class File;

struct Scan_files_param
{
	QString start_path;
};

class File_system_stat
{
public:
	File_system_stat();

	void update(const QString& path);

	uint64 full_disk_size() const { return full_disk_size_; }
	uint64 free_disk_size() const { return free_disk_size_; }

protected:

	uint64 full_disk_size_;
	uint64 free_disk_size_;
};

class Scan_manager : public QObject
{
	 Q_OBJECT

private:
	class Call_scan : public QThread
	{
	protected:
		Call_scan(Scan_manager* mgr) : mgr_(mgr), soft_stop_(false) {}
		~Call_scan() {}

		void set_params(const Scan_files_param& params) { params_ = params; }

		void run();

	protected:

		friend class Scan_manager; 

	private:
		Scan_manager* mgr_;
		bool soft_stop_;
		Scan_files_param params_;
	};

public:
	Scan_manager();
	~Scan_manager();

	static Scan_manager* i();

	void prepare_for_scan();

	void start_scan_thread(const Scan_files_param& param);

	void stop_scan_thread();

	bool is_scan_finished() const;

	const File_system* fs() const;

	const File_system_stat* fs_stat() const { return ready_? &stat_ : nullptr; }

	uint64 last_time_exec() const { return time_exec_; }

	uint64 approx_scan_time_left() const;

	Scanner& scanner() { return scanner_; }

	void flush();

signals:
	void scan_finished();

	void update_info();

public slots:
	void update_timer();

protected:
	void prepare_files(File* file);

	void scan(const Scan_files_param& param);

protected:
	static Scan_manager* instance_;

	File_system* fs_;
	Scanner scanner_;

	Call_scan thread_;

	bool ready_;

	File_system_stat stat_;

	uint64 time_exec_;
	uint64 start_time_;

	bool soft_stop_;
};

class Clean_manager
{
public:
	Clean_manager() {}
	~Clean_manager() {}

	enum Action 
	{
		MOVE_TO_RECYCLE_BIN,
		REMOVE
	};

	/// The most terrifying function
	void make_clean(const Action action);

protected:
	static void make_clean_rec(const File* file, Action action);

	static void move_file_to_recycle_bin(const File* file);
	static void remove_file(const File* file);
	static void remove_directory(const File* file);
};

template <typename T, typename S>
class Histogram
{
public:
	typedef std::vector<S> Bars_array;
	Bars_array bars_;

public:
	Histogram(const T& min, const T& max, const T& bar)
	:	min_(min), max_(max), bar_(bar)
	{
		size_t sz = size_t((max - min + bar - 1) / bar);
		bars_.resize(sz);
		memset(&bars_.front(), 0, sz * sizeof(S));
	}

	void add(const T& val)
	{
		bars_[select_bar(val)]++;
	}

	const Bars_array& get_all() const
	{
		return bars_;
	}

	S get(size_t i_bar) const
	{
		return bars_[i_bar];
	}

	const T& min() const { return min_; }
	const T& max() const { return max_; }
	const T& bar() const { return bar_; }

	T left(size_t i_bar)  const { return min_ + i_bar * bar_;}
	T right(size_t i_bar) const { return min_ + (i_bar + 1) * bar_;}

	size_t bars() const { return bars_.size(); }

protected:
	size_t select_bar(const T& val)
	{
		if (val < min_) return 0;
		if (val > max_) return bars_.size() - 1;
		return (val - min_) / bar_;
	}

protected:
	
	const T min_, max_;
	const T bar_;
};

class File_histogram_manager
{
public:
	typedef Histogram<uint64, uint32> File_hist;

public:
	File_histogram_manager();

	void go();

	const File_hist& histogram() const;

protected:
	void go(const File* file);

	const File_system* fs();

protected:
	
	File_hist hist_;
};

} // namespace xdd