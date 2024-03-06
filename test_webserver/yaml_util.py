import yaml


class YamlUtil:
    def __init__(self,yaml_file):
        self.yaml_file = yaml_file

    def read_yaml(self):
        with open(self.yaml_file,encoding = "utf-8") as file:
            value = yaml.load(file,Loader=yaml.FullLoader)
            return value

