package cn.ceopen.shard.utils;

import java.util.ArrayList;
import java.util.List;
import java.util.SortedMap;
import java.util.TreeMap;

import org.apache.commons.codec.digest.DigestUtils;

/**
 * The consistency of the hash implementation
 * @Author Xu Liangyong
 *
 * On January 22, 2015 at 2:34:05 PM
 */
public class ConsistantHash2 {

	//The ring with treemap is the main reason of can sort
	private TreeMap<Long, Node> circle = new TreeMap<Long, Node>();
	
	//The real node
	private List<Node> realNodes = new ArrayList<Node>();
	
	public static void main(String[] args) {
		ConsistantHash2 h = new ConsistantHash2();
		h.addNode(new Node(1));
		h.addNode(new Node(2));
//		h.addNode(new ConsistentHash.Node(3));
//		h.addNode(new ConsistentHash.Node(4));
//		h.addNode(new ConsistentHash.Node(5));
//		h.addNode(new ConsistentHash.Node(6));
//		h.addNode(new ConsistentHash.Node(7));
//		h.addNode(new ConsistentHash.Node(8));
//		h.addNode(new ConsistentHash.Node(9));
		
		for (int i = 0; i <50; i++) {
			h.getNode("" + i);
			//System.out.println(i + "--->" + h.getNode("" + i));
		}
	}
	
	
	public void addNode(Node node){
		realNodes.add(node);
		
		Long nodeKey = md5(node.toString());
		System.out.println(node.toString() + " md5:" + nodeKey);
		circle.put(nodeKey, node);
	}
	
	public void removeNode(Node node){
		realNodes.remove(node);
		
		Long nodeKey = md5(node.toString());
		circle.remove(nodeKey);
	}
	
	/**
	 * 
	 * @param key
	 * @return
	 */
	public Node getNode(String key){
		//Treemap into the good order of map
		Long keyMd5 = md5(key);
		SortedMap<Long, Node> sortedMap = circle.tailMap(keyMd5);
		Long k = null;
		
		//Don't choose the first hit
		if(sortedMap.isEmpty()){
			k = (circle.firstKey());
			System.out.println("not hit");
		}else{
			k = (sortedMap.firstKey());
		}
		Node node = circle.get(k);
		
		//Under normal circumstances, MD5(key) <md5(node)
		System.out.println(key + "(" + keyMd5 +") --->" + node.toString() + "(" + md5(node.toString()) + ")");
		return node;
	}
	
	private long md5(String key){
		byte[] bKey = DigestUtils.md5(key.getBytes());
		long res = ((long) (bKey[3] & 0xFF) <<24) | ((long) (bKey[2] & 0xFF) <<16) | ((long) (bKey[1] & 0xFF) <<8)| bKey[0] & 0xFF;  
	    return res;  
	}
	
	static class Node{
		private int nodeNum;
		
		public Node(int num){
			this.nodeNum = num;
		}
		
		@Override
		public String toString(){
			return "The real node:" + nodeNum ;
		}
	}
}





//tom white version
import java.util.Collection;
import java.util.SortedMap;
import java.util.TreeMap;

public class ConsistentHash<T> {

 private final HashFunction hashFunction;
 private final int numberOfReplicas;
 private final SortedMap<Integer, T> circle = new TreeMap<Integer, T>();

 public ConsistentHash(HashFunction hashFunction, int numberOfReplicas,
     Collection<T> nodes) {
   this.hashFunction = hashFunction;
   this.numberOfReplicas = numberOfReplicas;

   for (T node : nodes) {
     add(node);
   }
 }

 public void add(T node) {
   for (int i = 0; i < numberOfReplicas; i++) {
     circle.put(hashFunction.hash(node.toString() + i), node);
   }
 }

 public void remove(T node) {
   for (int i = 0; i < numberOfReplicas; i++) {
     circle.remove(hashFunction.hash(node.toString() + i));
   }
 }

 public T get(Object key) {
   if (circle.isEmpty()) {
     return null;
   }
   int hash = hashFunction.hash(key);
   if (!circle.containsKey(hash)) {
     SortedMap<Integer, T> tailMap = circle.tailMap(hash);
     hash = tailMap.isEmpty() ? circle.firstKey() : tailMap.firstKey();
   }
   return circle.get(hash);
 }

}
