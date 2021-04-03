
public class HelloWorld {
	
	private final String word = "Hello World!";
	
	public static void main(String[] args) {
		new HelloWorld().sayHello();
	}
	
	public void sayHello() {
		System.out.println(word);
	}
}