using System.ComponentModel;
using System.Reflection;

namespace CsCodeExample;

public enum Gender
{
    Neutrum = 0,
    Masculinum = 1,
    Femininum = 2
}

public enum Sex
{
    [Description("Male desc")] // Attributes
    Male = 0,
    [Description("Female desc")]
    Female = 1
}

public static class EnumEx
{
    public static String GetDescription(this Enum value)
    {
        FieldInfo field = value.GetType().GetField(value.ToString()); // Reflection
        DescriptionAttribute attribute = Attribute.GetCustomAttribute(field, typeof(DescriptionAttribute)) as DescriptionAttribute;
        return attribute == null ? value.ToString() : attribute.Description;
    }
}
