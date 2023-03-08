<template>
  <el-container>
    <div class="login">
      <el-card>
        <el-row
          style="
            justify-content: center;
            align-items: center;
            margin: 20px 0 0 0;
          "
        >
          <img src="static/logo_v.png" alt="logo" height="120" />
        </el-row>
        <el-row style="justify-content: center; align-items: center">
          <h2>注册</h2>
        </el-row>
        <el-form
          class="login-form"
          :model="model"
          ref="form"
          @submit.native.prevent="addUser"
        >
          <el-form-item prop="username">
            <el-input
              v-model="model.username"
              placeholder="用户名"
              clearable
            ></el-input>
          </el-form-item>
          <el-form-item prop="password1">
            <el-input
              v-model="model.password1"
              placeholder="密码"
              type="password"
              show-password
              clearable
            ></el-input>
          </el-form-item>
          <el-form-item prop="password2">
            <el-input
              v-model="model.password2"
              placeholder="确认密码"
              type="password"
              show-password
              clearable
            ></el-input>
          </el-form-item>




          <el-form-item>
            <el-button
              :loading="loading"
              class="login-button"
              type="primary"
              native-type="submit"
              block
              >注册</el-button
            >
          </el-form-item>
        </el-form>
        <el-row>
          <router-link to="/Login">
            <el-link>已有账号？点此登录</el-link>
          </router-link>
        </el-row>
      </el-card>
    </div>
  </el-container>
</template>
<script>
const { ElMessage } = ElementPlus;
export default {
  name: "register",
  data() {
    return {
      model: {
        username: "",
        password1: "",
        password2: "",
      },
      loading: false,

    };
  },
  methods: {
    addUser() {
          if(this.model.password1!=this.model.password2){
            ElMessage.error("两次密码不一致")
            return;
          }
          axios
            .post("/api/register", {
              user_name: this.model.username,
              password: this.model.password1
            })
            .then((response) => {
              console.log(response)
              if (response.data.message == "success") {
                ElMessage({
                  message: "注册成功",
                  type: "success",
                });
                this.$router.push("/Login");
              } else {
                ElMessage({
                  message: response.data.message,
                  type: "error",
                });
              }
            })
            .catch(function (error) {
              console.log(error);
            });
        
    },
  },
};
</script>
 
<style scoped>
.login {
  width: 100%;
  height: 100%;
  flex: 1;
  display: flex;
  justify-content: center;
  align-items: center;
  position: absolute;
}

.login-button {
  width: 100%;
  margin-top: 20px;
}
.login-form {
  width: 250px;
}
.forgot-password {
  margin-top: 10px;
}
</style>
<!-- <style lang="scss">
$teal: #094ab1;
.el-button--primary {
  background: $teal;
  border-color: $teal;

  &:hover,
  &.active,
  &:focus {
    background: lighten($teal, 7);
    border-color: lighten($teal, 7);
  }
}
.login .el-input__inner:hover {
  border-color: $teal;
}
.login .el-input__prefix {
  background: rgb(238, 237, 234);
  left: 0;
  height: calc(100% - 2px);
  left: 1px;
  top: 1px;
  border-radius: 3px;
  .el-input__icon {
    width: 30px;
  }
}
.login .el-input input {
  padding-left: 10px;
}
.login .el-card {
  padding-top: 0;
  padding-bottom: 30px;
}
h2 {
  font-family: "Open Sans";
  letter-spacing: 1px;
  font-family: Roboto, sans-serif;
  padding-bottom: 20px;
}
a {
  color: $teal;
  text-decoration: none;
  &:hover,
  &:active,
  &:focus {
    color: lighten($teal, 7);
  }
}
.login .el-card {
  width: 340px;
  display: flex;
  justify-content: center;
}
</style> -->