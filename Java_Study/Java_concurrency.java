import java.util.Arrays;
import java.util.Comparator;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;


public class CoreJavaForImpatientCh10Concurrency {
	public static void main(String[] args){
		//10.1 Concurrent Tasks
		Runnable hellos = () -> {
			for (int i = 1; i <= 10; i++){
				System.out.println("Hello " + i) ;
			}
		};
		Runnable goodbyes = () -> {
			for( int i = 1; i <= 10; i++){
				System.out.println("Goodbye " + i);
			}
		};
		Executor exec = Executors.newCachedThreadPool();
		exec.execute(hellos);
		exec.execute(goodbyes);
		
		//10.3
		//Parallel Algorithm
		String[] yetToSortedArray = {"Abc","hello world","nihao?","chilemaNin"};
		
		//java8
		Arrays.parallelSort(yetToSortedArray, Comparator.comparing(String::length)); // here, we are passing "method reference"  
		
//		Arrays.sort(yetToSortedArray, new Comparator<String>() { //java6
//			@Override
//			public int compare(String s1, String s2){
//				return ((Integer)s1.length()).compareTo(s2.length());
//			}
//		}); 
//		
//		Arrays.sort(yetToSortedArray, new Comparator<String>() { //java6
//			@Override
//			public int compare(String s1, String s2){
//				return Integer.compare(s1.length(), s2.length());
//			}
//		});
		for(String str : yetToSortedArray){
			System.out.println(str + ", ");
		}
		
		//10.4 Threadsafe Data Structures
		ConcurrentHashMap<String, Integer> map  = new ConcurrentHashMap<String, Integer>();
		map.put("Nihao",  5);
		map.put("Ciao",  4);
		map.compute("Nihao", (k,v) -> v == null ? 1 : v + 1); // the compute method is atomic.
		// Also : computeIfPresent , computeIfAbsent, putIfAbsent
		System.out.println("Nihao has " + map.get("Nihao") + " characters?");
		//BlockQueues:
		// put, take, add, remove, element,  [offer, poll, peek.]: most useful 3.
		//ConcurrentSkipListMap, ConcurrentSkipListSet
		
		
		//10.5 Atomic values
		AtomicLong nextNumber = new AtomicLong(3);
		//... in some thread...
		long id = nextNumber.incrementAndGet();
		System.out.println(id);
		
		
		//
		AtomicLong largest = new AtomicLong();
		int observed = 0;
		largest.updateAndGet(x-> Math.max(x, observed));
		// updateAndGet, getAndAccumulate
		
		
		//10.6 Locks
		//10.6.1 reentrant locks
		Lock countLock = new ReentrantLock(); //shared among multiple threads
		int count = 1;
		countLock.lock();
		try{
			count++; // critical section
		} finally {
			countLock.unlock();
		}
		
		//10.6.2 synchronized keyword
		// You don't have to use an explicit lock because in Java, every object has an intrinsic lock. 
		// The synchronized keyword is used to lock the intrinsic lock.
		
		// public synchronized int increment() { value++; return value;}
		
		//10.6.3 Waiting on Conditions.
		//wait() method --> the current thread is now deactivated and gives up the lock. (Avoid deadlock). It's in "wait-set"
		// it will keep deactivated until another thread has called the "notifyAll()" on the same object.
		
		
		//10.7 Threads
//		Runnable task = () -> {
//			...
//			Thread.sleep(millis); // makes the current thread sleep for a given period. So that some other threads ave a chance to do work.
//		}
//		
//		Thread thread = new Thread(task);
//		thread.start();
//		thread.join() // if you want to waif for a thread to finish, use join method.
		//10.7.2 Thread interruption.
		
//		
//		Runnable task = () -> {
//		    while (more work to do) {
//		        if (Thread.currentThread().isInterrupted()) return;
//		        Do more work
//		    }
//		};
		
		
		//10.8 Async computation.  
		// implement wait-free async computations.
		//User runLater() to start new thread. In jafaFx. other UI has similar call.
		//Because java UI is not thread safe (including JavaFX, Swing and Android)
		
		
		//10.8.2 Completable Futures. 
		//
	}
}







Book : java concurrency in practice:
all code: http://jcip.net.s3-website-us-east-1.amazonaws.com/listings.html

chapter 5.3 Blocking Queue, with consu-producer pattern.

package net.jcip.examples;

import java.io.File;
import java.io.FileFilter;
import java.util.concurrent.*;

/**
 * ProducerConsumer
 * <p/>
 * Producer and consumer tasks in a desktop search application
 *
 * @author Brian Goetz and Tim Peierls
 */
public class ProducerConsumer {
    static class FileCrawler implements Runnable {
        private final BlockingQueue<File> fileQueue;
        private final FileFilter fileFilter;
        private final File root;

        public FileCrawler(BlockingQueue<File> fileQueue,
                           final FileFilter fileFilter,
                           File root) {
            this.fileQueue = fileQueue;
            this.root = root;
            this.fileFilter = new FileFilter() {
                public boolean accept(File f) {
                    return f.isDirectory() || fileFilter.accept(f);
                }
            };
        }

        private boolean alreadyIndexed(File f) {
            return false;
        }

        public void run() {
            try {
                crawl(root);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }

        private void crawl(File root) throws InterruptedException {
            File[] entries = root.listFiles(fileFilter);
            if (entries != null) {
                for (File entry : entries)
                    if (entry.isDirectory())
                        crawl(entry);
                    else if (!alreadyIndexed(entry))
                        fileQueue.put(entry);
            }
        }
    }

    static class Indexer implements Runnable {
        private final BlockingQueue<File> queue;

        public Indexer(BlockingQueue<File> queue) {
            this.queue = queue;
        }

        public void run() {
            try {
                while (true)
                    indexFile(queue.take());
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }

        public void indexFile(File file) {
            // Index the file...
        };
    }

    private static final int BOUND = 10;
    private static final int N_CONSUMERS = Runtime.getRuntime().availableProcessors();

    public static void startIndexing(File[] roots) {
        BlockingQueue<File> queue = new LinkedBlockingQueue<File>(BOUND);
        FileFilter filter = new FileFilter() {
            public boolean accept(File file) {
                return true;
            }
        };

        for (File root : roots)
            new Thread(new FileCrawler(queue, filter, root)).start();

        for (int i = 0; i < N_CONSUMERS; i++)
            new Thread(new Indexer(queue)).start();
    }
}







