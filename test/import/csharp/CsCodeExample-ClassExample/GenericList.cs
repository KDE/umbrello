namespace CsCodeExample;

public interface InitInterface
{
    void Init(object[] array);
}

public class GenericTest<T> where T : InitInterface, new()
{
    public static List<T> Run()
    {
        List<T> list = new List<T>();
        var t = new T();
        t.Init(new object[2]);
        list.Add(t);
        return list;
    }
}

public class A : InitInterface
{
    public int X { get; set; }

    public void Init(object[] array)
    {
        X = (int)array[0];
    }
}

public class B : InitInterface
{
    public double Y { get; set; }

    public void Init(object[] array)
    {
        Y = (double)array[0];
    }
}
