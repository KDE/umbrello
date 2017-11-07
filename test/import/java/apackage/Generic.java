package apackage;

public class Generic<T> {
	private T value;
	public Generic() {}
	public Generic(T value) { this.value = value; }
	public void set(T value) { this.value = value; }
	public T get() { return value; }
	public boolean isEmpty() { return value != null; }
	public void empty() { value = null; }
}
