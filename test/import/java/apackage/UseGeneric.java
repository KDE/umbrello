package apackage;

public class UseGeneric {
	private Generic<Integer> _a;
	public UseGeneric(Generic<Integer> a) {}
	public Generic<Integer> getA() {
		return _a;
	}
	public void setA(Generic<Integer> _a) {
		this._a = _a;
	}
}
