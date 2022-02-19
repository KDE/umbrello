/**
 * Tests the following CSharpImport features:
 * Properties https://bugs.kde.org/show_bug.cgi?id=375223
 * Mapping of array of primitive type to predefined datatype
 * Mapping of .NET alias type to native C# type
 */
public class Properties
{
   public int           property1 { get; set; }

   // map to array type in Umbrello "Datatypes" folder (without creating a new type)
   public string[]      property2 { get; set; }

   // map to type "nint" in Umbrello "Datatypes" folder (without creating a new type)
   public System.IntPtr property3 { get; set; }

   // Variation of property syntax https://www.w3schools.com/cs/cs_properties.php
   private string name;  // field
   public string Name    // property
   {
     get { return name; }
     set { name = value; }
   }
}
