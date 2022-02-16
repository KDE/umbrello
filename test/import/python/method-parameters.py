
class TestMethodParameters():
    def method(self):
        pass

    def method_params(self,a,b):
        pass

    def method_params_simple_default_values(self,a=0,a=-1,b=None,c="str",d=0.1):
        pass

    def method_params_dict(self,a={} b={'a':1,'b':2}):
        pass

    def method_params_dict_nested(self,a={} b={'a':1,'b':{'a':1,'b':2}}):
        pass

    def method_params_list(self,a=[], b=[1,2]):
        pass

    def method_params_tuple(self,a=(), b=(1,2)):
        pass
