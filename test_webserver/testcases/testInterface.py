import pytest
import requests

from yaml_util import YamlUtil


class TestInterface(object):
    step = 0

    # 测试注册页面 注册失败
    @pytest.mark.parametrize('args', YamlUtil("test_interface.yaml").read_yaml())
    def test_register_fail(self, args):
        url = args['request'][TestInterface.step]['url']
        method = args['request'][TestInterface.step]['method']
        headers = args['request'][TestInterface.step]['headers']
        data = args['request'][TestInterface.step]['data']
        TestInterface.step += 1

        res = requests.request(method, url, headers=headers,data=data)
        assert res.status_code == 200 and "提示：该用户名被注册." in res.text

    # 测试注册页面 注册成功
    @pytest.mark.parametrize('args', YamlUtil("test_interface.yaml").read_yaml())
    def test_register_succeed(self, args):
        url = args['request'][TestInterface.step]['url']
        method = args['request'][TestInterface.step]['method']
        headers = args['request'][TestInterface.step]['headers']
        data = args['request'][TestInterface.step]['data']
        TestInterface.step += 1

        res = requests.request(method, url, headers=headers, data=data)
        assert res.status_code == 200 and "登录" in res.text

    # 测试登录页面 登录失败
    @pytest.mark.parametrize('args', YamlUtil("test_interface.yaml").read_yaml())
    def test_login_fail(self, args):
        url = args['request'][TestInterface.step]['url']
        method = args['request'][TestInterface.step]['method']
        headers = args['request'][TestInterface.step]['headers']
        data = args['request'][TestInterface.step]['data']
        TestInterface.step += 1

        res = requests.request(method, url, headers=headers, data=data)
        assert res.status_code == 200 and "提示：用户名或密码错误，请重试" in res.text

    # 测试登录页面 登录成功
    @pytest.mark.parametrize('args', YamlUtil("test_interface.yaml").read_yaml())
    def test_login_succeed(self, args):
        url = args['request'][TestInterface.step]['url']
        method = args['request'][TestInterface.step]['method']
        headers = args['request'][TestInterface.step]['headers']
        data = args['request'][TestInterface.step]['data']
        TestInterface.step += 1

        res = requests.request(method, url, headers=headers, data=data)
        assert res.status_code == 200 and "是时候做出选择了" in res.text

    # 测试图片资源
    @pytest.mark.parametrize('args', YamlUtil("test_interface.yaml").read_yaml())
    def test_request_picture(self, args):
        url = args['request'][TestInterface.step]['url']
        method = args['request'][TestInterface.step]['method']
        headers = args['request'][TestInterface.step]['headers']
        TestInterface.step += 1

        res = requests.request(method, url, headers=headers)
        assert res.status_code == 200 and ".jpg" in res.text

    # 测试视频资源
    @pytest.mark.parametrize('args', YamlUtil("test_interface.yaml").read_yaml())
    def test_request_video(self, args):
        url = args['request'][TestInterface.step]['url']
        method = args['request'][TestInterface.step]['method']
        headers = args['request'][TestInterface.step]['headers']
        TestInterface.step += 1

        res = requests.request(method, url, headers=headers)
        assert res.status_code == 200 and ".mp4" in res.text
