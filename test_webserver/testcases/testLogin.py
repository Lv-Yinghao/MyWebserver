from testcases.index_page import IndexTask
from testcases.login_page import LoginTask
from utils import DriverUtil
import pytest

from yaml_util import YamlUtil


class TestLogin:
    step = 0

    def setup_method(self):
        self.driver.get("http://192.168.179.128:9006")
        self.index_task.go_to_login()

    def setup_class(self):
        self.driver = DriverUtil.getDriver()
        self.index_task = IndexTask()
        self.login_task = LoginTask()

    def teardown_class(self):
        self.driver = DriverUtil.quitDriver()

    # 登录失败测试用例
    @pytest.mark.parametrize('args', YamlUtil("test_login.yaml").read_yaml())
    def test_login_fail(self, args):
        self.login_task.login_method(args['login'][TestLogin.step]['username'],
                                     args['login'][TestLogin.step]['password'])
        TestLogin.step += 1
        assert self.login_task.judge_login_fail()

    # 登录成功测试用例(登录成功后跳转到主页面)
    @pytest.mark.parametrize('args', YamlUtil("test_login.yaml").read_yaml())
    def test_login_succeed(self, args):
        self.login_task.login_method(args['login'][TestLogin.step]['username'],
                                     args['login'][TestLogin.step]['password'])
        TestLogin.step += 1
        assert self.login_task.judge_login_succeed()

    # 用户名为空测试用例 通过获取元素属性判断
    def test_username_is_empty(self):
        assert self.login_task.judge_user_empty()

    # 密码为空测试用例 通过获取元素属性判断
    @pytest.mark.parametrize('args', YamlUtil("test_login.yaml").read_yaml())
    def test_password_is_empty(self, args):
        self.login_task.login_method(args['login'][TestLogin.step]['username'],
                                     args['login'][TestLogin.step]['password'])
        TestLogin.step += 1
        assert self.login_task.judge_password_empty()

    # 用户名过长测试用例
    @pytest.mark.parametrize('args', YamlUtil("test_login.yaml").read_yaml())
    def test_username_too_long(self, args):
        self.login_task.login_method(args['login'][TestLogin.step]['username'],
                                     args['login'][TestLogin.step]['password'])
        TestLogin.step += 1
        assert self.login_task.judge_username_too_long()

    # 用户名过短测试用例
    @pytest.mark.parametrize('args', YamlUtil("test_login.yaml").read_yaml())
    def test_username_too_short(self, args):
        self.login_task.login_method(args['login'][TestLogin.step]['username'],
                                     args['login'][TestLogin.step]['password'])
        TestLogin.step += 1
        assert self.login_task.judge_username_too_short()

    # 密码过长测试用例
    @pytest.mark.parametrize('args', YamlUtil("test_login.yaml").read_yaml())
    def test_password_too_long(self, args):
        self.login_task.login_method(args['login'][TestLogin.step]['username'],
                                     args['login'][TestLogin.step]['password'])
        TestLogin.step += 1
        assert self.login_task.judge_password_too_long()

    # 密码过短测试用例
    @pytest.mark.parametrize('args', YamlUtil("test_login.yaml").read_yaml())
    def test_password_too_short(self, args):
        self.login_task.login_method(args['login'][TestLogin.step]['username'],
                                     args['login'][TestLogin.step]['password'])
        TestLogin.step += 1
        assert self.login_task.judge_password_too_short()

    # 密码加密测试用例
    def test_password_encryption(self):
        assert self.login_task.judge_password_encryption()

    # 登录失败后清空输入框测试用例
    @pytest.mark.parametrize('args', YamlUtil("test_login.yaml").read_yaml())
    def test_clear_inputBox(self, args):
        self.login_task.login_method(args['login'][TestLogin.step]['username'],
                                     args['login'][TestLogin.step]['password'])
        TestLogin.step += 1
        assert self.login_task.judge_clear_inputBox()

    """易用性测试"""

    # 使用Tab和Enter键完成登录测试用例 通过切换页面焦点实现 比较特殊
    def test_automated_login(self):
        assert self.login_task.judge_automated_login()

    """"安全性测试"""

    # SQL注入测试用例
    @pytest.mark.parametrize('args', YamlUtil("test_login.yaml").read_yaml())
    def test_sql_injection(self, args):
        self.login_task.login_method(args['login'][TestLogin.step]['username'],
                                     args['login'][TestLogin.step]['password'])
        TestLogin.step += 1
        assert self.login_task.judge_login_fail()

    # XSS攻击测试用例
    @pytest.mark.parametrize('args', YamlUtil("test_login.yaml").read_yaml())
    def test_xss_attack(self, args):
        self.login_task.login_method(args['login'][TestLogin.step]['username'],
                                     args['login'][TestLogin.step]['password'])
        TestLogin.step += 1
        assert self.login_task.judge_username_too_long()
