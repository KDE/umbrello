namespace CsCodeExample;

public interface Employee
{
    decimal Sallary { get; set; }
    string Company { get; set; }
    int YearsOfService { get; set; }

    decimal CalculateSallaryCoeficiente();
}

//public partial class User // keyword: partial
public class User : Person, Employee // Inheritance Class (Parent - Child), Implements Interface
{
    public string UserName { get; set; }
    public string Email { get; set; }
    protected string Password { get; set; }

    public string Company { get; set; }
    public int YearsOfService { get; set; }
    public decimal Sallary { get; set; }

    public void ResetPassword(string pass)
    {
        if (pass.Length < 8)
            throw new Exception("Password must have at least 8 characters.");
        Password = pass;
    }

    public override string ToString()
    {
        return String.Format("{0} ({1})", base.ToString(), Company);
    }

    public decimal CalculateSallaryCoeficiente()
    {
        if (YearsOfService < 10)
            return 1;
        else
            return 1.5m;
    }
}
