namespace CsCodeExample;

public class ComplexNumber // class
{
    private double real; // Fields (usually hidden with properties)

    public double Real // Properties ( Encapsulation, auto generated - Encapsulate field )
    {
        get { return real; }
        set { real = value; }
    }

    public double Imag { get; set; } // Property with hidded field, PascalCase
    public static string Format { get { return "a + b * i"; } }

    protected const double PI = 3.14; // keyword: const (just for example, in this class Math.PI is used because it has more precision)

    public ComplexNumber() { } // class constructor, default - no parameters

    public ComplexNumber(double real, double imag = 0) // constructor with 2 parameters, second has default value 
    {
        Real = real;
        Imag = imag;
    }

    public ComplexNumber Add(ComplexNumber input)
    {
        var result = new ComplexNumber
        {
            Real = Real + input.Real,
            Imag = Imag + input.Imag
        };
        return result;
    }

    public static ComplexNumber Add(ComplexNumber a, ComplexNumber b)
    {
        var result = new ComplexNumber
        {
            Real = a.Real + b.Real,
            Imag = b.Imag + b.Imag
        };
        return result;
    }

    public ComplexNumber Subtract(ComplexNumber input)
    {
        var result = new ComplexNumber
        {
            Real = Real - input.Real,
            Imag = Imag - input.Imag
        };
        return result;
    }

    public ComplexNumber Multiply(ComplexNumber input)
    {
        var result = new ComplexNumber
        {
            Real = Real * input.Real - Imag * input.Imag,
            Imag = Real * input.Imag + Imag * input.Real
        };
        return result;
    }

    public ComplexNumber Divide(ComplexNumber input)
    {
        var result = new ComplexNumber
        {
            Real = (Real * input.Real + Imag * input.Imag) / (input.Real * input.Real + input.Imag * input.Imag),
            Imag = (Real * input.Imag - Imag * input.Real) / (input.Real * input.Real + input.Imag * input.Imag)
        };
        return result;
    }

    public double Modul ()
    {
        return Math.Sqrt(Real * Real + Imag * Imag);
    }

    public ComplexNumber Pow(int n)
    {
        var result = new ComplexNumber(1, 0);
        if (n != 0)
        {
            for (int i = 0; i < Math.Abs(n); i++)
            {
                result = result.Multiply(this);
            }
            if (n < 0)
            {
                result = new ComplexNumber(1, 0).Divide(result);
            }
        }
        return result;
    }

    public List<ComplexNumber> Nroot(int n)
    {
        var result = new List<ComplexNumber>();
        double r = Modul();
        double f = Math.Atan(Imag / Real);
        if (Real == 0.0)
        {
            f = Math.PI/2;
            if (Imag < 0)
            {
                f += Math.PI;
            }
        }
        if (Real < 0)
        {
            f += Math.PI;
        }
        double p1 = Math.Pow(r, n);

        for(var i = 0; i < n; i++)
        {
            double p2 = (f + 2 * i * Math.PI) / n;
            result.Add(new ComplexNumber(p1 * Math.Cos(p2), p1 * Math.Sin(p2)));
        }
        return result;
    }

    public override string ToString() // keyword: override
    {
        return string.Format("({0:0.00}, {1:0.00})", Real, Imag);
    }

    public string ToCSV() // CSV - Comma Separated Value
    {
        return string.Format("{0:0.00},{1:0.00}", Real, Imag);
    }

    public virtual string ToDiffString() // keyword: virtual(can be overridden in child class)
    {
        string output = "";
        if (Imag == 0.0)
        {
            output = string.Format("{0}", Real);
        }
        else if (Imag > 0)
        {
            output = string.Format("{0} + {1}i", Real, Imag);
        }
        else if (Imag < 0)
        {
            output = string.Format("{0} - {1}i", Real, -Imag);
        }
        return output;
     }

    public static string GetDescription()
    {
        return "This class implements basic math function with complex numbers.";
    }
}

public class MoreComplexNumber : ComplexNumber
{
    public MoreComplexNumber() : base() { }

    public MoreComplexNumber(double real, double imag = 0) : base(real, imag) { }

    //public MoreComplexNumber(double real, double imag = 0)
    //{
    //    Real = real;
    //    Imag = imag;
    //}

    public override string ToDiffString()
    {
        string output = "";
        if (Imag == 0.0)
        {
            output = String.Format("{0}", Real);
        }
        else if (Imag > 0)
        {
            output = String.Format("{0} + {1} * i", Real, Imag);
        }
        else if (Imag < 0)
        {
            output = String.Format("{0} - {1} * i", Real, -Imag);
        }
        return output;
    }
}
