package apackage;

class Derived extends Base {
	public Derived() {}

	public void publicMethod() {}
	protected void protectedMethod() {}
	@SuppressWarnings("unused")
	private void privateMethod() {}

	static public void staticPublicMethod() {}
	static protected void staticProtectedMethod() {}
	@SuppressWarnings("unused")
	static private void staticPrivateMethod() {}

	int publicMethodWithParameterAndReturnType(int a) { return a; }

	public int _public_int_member;
	protected int _protected_int_member;
	@SuppressWarnings("unused")
	private int _private_int_member;
	@SuppressWarnings("unused")
	private int _private_initialized_int_member = 0;
	static int _static_int_member = 0;
	@SuppressWarnings("unused")
	private final int _private_final_int_member = 0;
}
