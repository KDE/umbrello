// file: helloWorld.vala
// source: https://orgmode.org/worg/org-contrib/babel/languages/ob-doc-vala.html

class Demo.HelloWorld : GLib.Object {
    public static int main(string[] args) {
        // skip args[0] as it is the binary name
        for (int i=1; i < args.length; i++) {
            stdout.printf("argv[%d] = %s\n" , i, args[i]);
        }
        return 0;
    }
}

