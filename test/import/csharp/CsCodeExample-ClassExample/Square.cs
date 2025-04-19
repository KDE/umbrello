namespace CsCodeExample;

abstract class Shape // keyword: abstract
{
    public abstract double CalcArea();
}

class Square : Shape
{
    protected double side; // AccessModifier: protected

    public Square(double side)
    {
        this.side = side; // keyword: this (to distinct field from local variable when they have same name)
    }

    public override double CalcArea()
    {
        return side * side;
    }
}
