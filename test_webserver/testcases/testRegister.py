import pytest

from testcases.index_page import IndexTask
from testcases.register_page import RegisterTask
from utils import DriverUtil
from yaml_util import YamlUtil


class TestRegister:
    step = 0

    def setup_method(self):
        self.driver.get("http://192.168.179.128:9006")
        self.index_task.go_to_register()

    def setup_class(self):
        self.driver = DriverUtil.getDriver()
        self.index_task = IndexTask()
        self.register_task = RegisterTask()

    def teardown_class(self):
        self.driver = DriverUtil.quitDriver()

    # 注册失败测试用例
    @pytest.mark.parametrize('args', YamlUtil("test_register.yaml").read_yaml())
    def test_register_fail(self, args):
        self.register_task.register_method(args['register'][TestRegister.step]['username'],
                                           args['register'][TestRegister.step]['password'])
        TestRegister.step += 1
        assert self.register_task.judge_register_fail()

    # 注册成功测试用例(注册成功后跳转到主页面)
    @pytest.mark.parametrize('args', YamlUtil("test_register.yaml").read_yaml())
    def test_register_succeed(self, args):
        username = args['register'][TestRegister.step]['username']
        password = args['register'][TestRegister.step]['password']
        TestRegister.step += 1
        self.register_task.register_method(username, password)
        assert self.register_task.judge_register_succeed(username, password)

    # 用户名为空测试用例 通过获取元素属性判断
    def test_username_is_empty(self):
        assert self.register_task.judge_user_empty()

    # 密码为空测试用例 通过获取元素属性判断
    @pytest.mark.parametrize('args', YamlUtil("test_register.yaml").read_yaml())
    def test_password_is_empty(self, args):
        self.register_task.register_method(args['register'][TestRegister.step]['username'],
                                           args['register'][TestRegister.step]['password'])
        TestRegister.step += 1
        assert self.register_task.judge_password_empty()

    # 用户名过长测试用例
    @pytest.mark.parametrize('args', YamlUtil("test_register.yaml").read_yaml())
    def test_username_too_long(self, args):
        self.register_task.register_method(args['register'][TestRegister.step]['username'],
                                           args['register'][TestRegister.step]['password'])
        TestRegister.step += 1
        assert self.register_task.judge_username_too_long()

    # 用户名过短测试用例
    @pytest.mark.parametrize('args', YamlUtil("test_register.yaml").read_yaml())
    def test_username_too_short(self, args):
        self.register_task.register_method(args['register'][TestRegister.step]['username'],
                                           args['register'][TestRegister.step]['password'])
        TestRegister.step += 1
        assert self.register_task.judge_username_too_short()

    # 密码过长测试用例
    @pytest.mark.parametrize('args', YamlUtil("test_register.yaml").read_yaml())
    def test_password_too_long(self, args):
        self.register_task.register_method(args['register'][TestRegister.step]['username'],
                                           args['register'][TestRegister.step]['password'])
        TestRegister.step += 1
        assert self.register_task.judge_password_too_long()

    # 密码过短测试用例
    @pytest.mark.parametrize('args', YamlUtil("test_register.yaml").read_yaml())
    def test_password_too_short(self, args):
        self.register_task.register_method(args['register'][TestRegister.step]['username'],
                                           args['register'][TestRegister.step]['password'])
        TestRegister.step += 1
        assert self.register_task.judge_password_too_short()

    # 密码加密测试用例
    def test_password_encryption(self):
        assert self.register_task.judge_password_encryption()

    # 注册失败后清空输入框测试用例
    @pytest.mark.parametrize('args', YamlUtil("test_register.yaml").read_yaml())
    def test_clear_inputBox(self, args):
        self.register_task.register_method(args['register'][TestRegister.step]['username'],
                                           args['register'][TestRegister.step]['password'])
        TestRegister.step += 1
        assert self.register_task.judge_clear_inputBox()

    """易用性测试"""

    # 使用Tab和Enter键完成注册测试用例 通过切换页面焦点实现 比较特殊
    def test_automated_register(self):
        assert self.register_task.judge_automated_register()

    """"安全性测试"""

    # SQL注入测试用例
    @pytest.mark.parametrize('args', YamlUtil("test_register.yaml").read_yaml())
    def test_sql_injection(self, args):
        self.register_task.register_method(args['register'][TestRegister.step]['username'],
                                           args['register'][TestRegister.step]['password'])
        TestRegister.step += 1
        assert self.register_task.judge_register_fail()

    # XSS攻击测试用例
    @pytest.mark.parametrize('args', YamlUtil("test_register.yaml").read_yaml())
    def test_xss_attack(self, args):
        self.register_task.register_method(args['register'][TestRegister.step]['username'],
                                           args['register'][TestRegister.step]['password'])
        assert self.register_task.judge_username_too_long()
