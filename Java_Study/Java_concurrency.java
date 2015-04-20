import java.util.Arrays;
import java.util.Comparator;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;


public class CoreJavaForImpatientCh10Concurrency {
	public static void main(String[] args){
		//10.1 Concurrent Tasks
		Runnable hellos = () -> {
			for (int i = 1; i <= 1000; i++){
				System.out.println("Hello " + i) ;
			}
		};
		Runnable goodbyes = () -> {
			for( int i = 1; i <= 1000; i++){
				System.out.println("Goodbye " + i);
			}
		};
		Executor exec = Executors.newCachedThreadPool();
		exec.execute(hellos);
		exec.execute(goodbyes);
		
		
		//10.3
		//Parallel Algorithm
		String[] yetToSortedArray = {"Abc","hello world","nihao?","chilemaNin"};
		Arrays.parallelSort(yetToSortedArray, Comparator.comparing(String::length));  //java8
		
		Arrays.sort(yetToSortedArray, new Comparator<String>() { //java6
			@Override
			public int compare(String s1, String s2){
				return ((Integer)s1.length()).compareTo(s2.length());
			}
		}); 
		
		Arrays.sort(yetToSortedArray, new Comparator<String>() { //java6
			@Override
			public int compare(String s1, String s2){
				return Integer.compare(s1.length(), s2.length());
			}
		});
		
		for(String str : yetToSortedArray){
			System.out.println(str + ", ");
		}
	}
}
