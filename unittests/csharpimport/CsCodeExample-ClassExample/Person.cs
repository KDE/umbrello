namespace org.kde.umbrello.unittests.codeimport.CsCodeExample;

public class Address
{
    public string City { get; set; }
    public string Street { get; set; }
    public int Number { get; set; }
}

public abstract class Human // keyword: abstract, can not be initialised, only inhereted
{
    public DateTime BirthDate { get; set; }
}

public class Person : Human
{
    public string FirstName { get; set; }
    public string LastName { get; set; }
    public Sex PersonSex { get; set; }
    public Address HomeAddress { get; set; } // Composition

    public int Age { get { return DateTime.Now.Year - BirthDate.Year; } }
    public string DisplayName { get { return FirstName + " " + LastName; } }

    public override string ToString()
    {
        return String.Format("{0} {1}", FirstName, LastName);
    }
}
