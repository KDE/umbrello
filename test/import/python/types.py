# coding=UTF-8

# see https://docs.python.org/3/tutorial/classes.html#class-and-instance-variables

class ReferencedType():
    def __init__(self, a=0, b=0):
        pass

myobject = ReferencedType()

class Types():
  class_variable_bool_true=True
  class_variable_bool_false=False
  class_variable_int=3
  class_variable_negative_int=-54341
  class_variable_float=3.12334
  class_variable_negative_float=-1.0876544
  class_variable_string="1234"
  class_variable_list=[]
  class_variable_list_params=[1,2]
  class_variable_dict={}
  class_variable_dict_params={'a':1, 'b':2}
  class_variable_tuple=()
  class_variable_tuple_params=(1,2,3)
  class_variable_object=myobject
  class_variable_object_type=ReferencedType()
  class_variable_object_type_params=ReferencedType(1,2)

  # We import instance variables only from __init__()
  def __init__():
    self.instance_variable_bool_true=True
    self.instance_variable_bool_false=False
    self.instance_variable_int=3
    self.instance_variable_negative_int=-54341
    self.instance_variable_float=3.12334
    self.instance_variable_negative_float=-1.9876
    self.test() # should be ignored
    self.instance_variable_string="1234"
    self.instance_variable_list=[]
    self.instance_variable_list_params=[1,2]
    self.instance_variable_dict={}
    self.instance_variable_dict_params={'a':1, 'b':2}
    self.instance_variable_tuples=()
    self.instance_variable_tuple_params=(1,2,3)
    self.instance_variable_object=myobject
    self.instance_variable_object_type=ReferencedType()
    self.instance_variable_object_type_params=ReferencedType(1,2)

  def test():
    pass
