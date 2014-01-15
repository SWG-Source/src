#ifndef _DB_TASK_REQUEST_H
#define _DB_TASK_REQUEST_H

namespace DB
{
	
class Session;
	
/** Abstract base class representing a request sent to the database task queue.
 */
class TaskRequest
{
  private:
	/** Flag to avoid calling Process() twice unexpectedly.
	 */
	bool needsProcessing;

	/** Error count to prevent infinite retries of an operation that always fails.
	 */
	int errorCount;
	int reconnectErrorCount;
	
	/** Called by the database thread.  Will invoke Process().  Do not override.
	 */
	void workerThreadLoop(DB::Session *session);

	/** Called by the main thread.  Will invoke IsFinished().  Do not override.
	 */
	bool mainThreadFinish();

	/** Called by the main thread.  Will invoke OnComplete().  Do not override.
	 */
	void mainThreadOnComplete();
	
  public:
	TaskRequest();
	virtual ~TaskRequest();
	
	/** Called by the database thread.  Should do whatever database work
	 * is needed to handle the request.
	 *
	 * This will only get executed once, unless IsFinished() returns false.
	 */
	virtual bool process(DB::Session *session)=0;

	/** Called by the main thread after Process() has completed.  Should check the
	 * results.  It should return true if the task is done, or false if the task
	 * should be put back on the queue for a worker thread to do more processing.
	 *
	 * It is not guaranteed that OnComplete() will be invoked if IsFinished() returns
	 * true, because a subtask's or parent task's IsFinished() might return false,
	 * requiring the whole set of tasks to be put back on the queue for the worker thread.
	 * Therefore, IsFinished() could be called again even after it returns true.  (And, since
	 * things may have changed in the mean time, it may need to do its checks again.)
	 *
	 * The default implementation does nothing and returns true.
	 */
	virtual bool isFinished(); 
		
	/** Called by the main thread (from DB::TaskQueue::Update()) after the request
	 * has been completed and IsFinished() has returned true.  OnComplete() will only
	 * be invoked once.
	 *
	 * It is guaranteed that if OnComplete() is invoked at all, it will be invoked immediately
	 * after IsFinished() returns true.  The only things that may happen between IsFinished()
	 * and OnComplete() is IsFinished() or OnComplete() on another task.
	 *
	 * However, it is not guaranteed that OnComplete() will be invoked at all when IsFinished()
	 * returns true, because subtasks might require more procssing.
	 */
	virtual void onComplete()=0;

	friend class TaskQueue;
};

} 
#endif
