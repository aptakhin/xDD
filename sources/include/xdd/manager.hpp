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
	String start_path;
};

class File_system_stat
{
public:
	File_system_stat();

	void update(const File_system* fs);

	uint64 full_disk_size() const { return _full_disk_size; }
	uint64 free_disk_size() const { return _free_disk_size; }

protected:

	uint64 _full_disk_size;
	uint64 _free_disk_size;
};

class Scan_manager : public QObject
{
	 Q_OBJECT

private:
	class Call_scan : public QThread
	{
	protected:
		Call_scan(Scan_manager* mgr) : _mgr(mgr) {}
		~Call_scan() {}

		void set_params(const Scan_files_param& params) { _params = params; }

		void run();

	protected:

		friend class Scan_manager; 

	private:
		Scan_manager* _mgr;
		Scan_files_param _params;
	};

public:
	Scan_manager();
	~Scan_manager();

	static Scan_manager* i();

	void prepare_for_scan();

	void start_scan_thread(const Scan_files_param& param);

	bool is_scan_finished() const;

	const File_system* fs() const;

	const File_system_stat* fs_stat() const { return _ready? &_stat : nullptr; }

	uint64 last_time_exec() const { return _time_exec; }

	Scanner& scanner() { return _scanner; }

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
	static Scan_manager* _instance;

	File_system* _fs;
	Scanner _scanner;

	Call_scan _thread;

	bool _ready;

	File_system_stat _stat;

	uint64 _time_exec;
};

class Clean_manager
{
public:
	Clean_manager() {}
	~Clean_manager() {}

	enum Action 
	{
		A_MOVE_TO_RECYCLE_BIN,
		A_REMOVE
	};

	/// The most terrifying function
	void make_clean(const Action action);

protected:
	void make_clean_rec(const File* file, Action action);
	void make_clean_file(const File* file, Action action);

	void move_file_to_recycle_bin(const File* file);
	void remove_file(const File* file);
	void remove_directory(const File* file);

	File_system* fs();
};

template <typename T, typename S>
class Histogram
{
public:
	typedef std::vector<S> Bars_array;
	Bars_array _bars;

public:
	Histogram(const T& min, const T& max, const T& bar)
	:	_min(min), _max(max), _bar(bar)
	{
		size_t sz = size_t((max - min + bar - 1) / bar);
		_bars.resize(sz);
		memset(&_bars.front(), 0, sz * sizeof(S));
	}

	void add(const T& val)
	{
		_bars[select_bar(val)]++;
	}

	const Bars_array& get_all() const
	{
		return _bars;
	}

	S get(size_t i_bar) const
	{
		return _bars[i_bar];
	}

	const T& min() const { return _min; }
	const T& max() const { return _max; }
	const T& bar() const { return _bar; }

	T left(size_t i_bar)  const { return _min + i_bar * _bar;}
	T right(size_t i_bar) const { return _min + (i_bar + 1) * _bar;}

	size_t bars() const { return _bars.size(); }

protected:
	size_t select_bar(const T& val)
	{
		if (val < _min) return 0;
		if (val > _max) return _bars.size() - 1;
		return (val - _min) / _bar;
	}

protected:
	
	const T _min, _max;
	const T _bar;
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
	
	File_hist _hist;
};

} // namespace xdd