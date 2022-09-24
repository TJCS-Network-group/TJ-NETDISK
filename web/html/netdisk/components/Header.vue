<template>
  <el-row class="header-row">
    <el-col :span="6">
      <router-link to="/UserSpace" tag="div">
      <img src="static/logo_h.png" alt="logo" height="40" />
      </router-link>
    </el-col>
    <el-col :span="15">
      <el-row align="middle">
        <slot></slot>
      </el-row>
    </el-col>
    <el-col :span="3">
      <el-row justify="end" align="middle">
        <el-avatar :size="40" :src="avatar" />
        <el-dropdown trigger="click" @command="handleCommand">
          <span
            class="el-dropdown-link"
            trigger="click"
            style="cursor: pointer; margin-left: 8px; font-weight: bold"
          >
            {{ username }}<i class="el-icon-arrow-down el-icon--right"></i>
          </span>
          <template #dropdown>
            <el-dropdown-menu>
              <!-- <el-dropdown-item command="revise">修改信息</el-dropdown-item>
              <el-dropdown-item divided command="space"
                >我的空间</el-dropdown-item
              > -->
              <el-dropdown-item divided command="logout"
                >退出登录</el-dropdown-item
              >
            </el-dropdown-menu>
          </template>
        </el-dropdown>
      </el-row>
    </el-col>
  </el-row>
</template>


<script>
export default {
  name: "Header",
  data() {
    return {
      username: "",
      uid:"",
    };
  },
  props:{
    avatar: {
		  type: String,
		  required: false,
		  default: 'static/default_avatar.jpg',
	  },
  },
  created() {
    axios.get("/api/myinfo/", {}).then((response) => {
      console.log(response)
      this.username = response.data.data.user_name;
      this.uid = response.data.data.uid;
    });
  },
  methods: {
    handleCommand(command) {
      switch (command) {
        case "revise":
          this.$router.push({ name: "Revise" });
          break;
        case "space":
          this.$router.push({ name: "UserSpace" });
          break;
        default:
          this.confirmLogout();
      }
    },
    avatarSrc(){
      return `/static/avatar/${this.uid}.jpg?${Date.now()}`;
    },
    confirmLogout() {
      this.$confirm("确认退出登录?", "提示", {
        confirmButtonText: "确定",
        cancelButtonText: "取消",
        type: "warning",
      }).then(() => {
        axios
          .get("/api/logout/")
          .then((response) => {
            console.log("logout response",response)
            if (response.data.message == "success") {
              this.$router.push({ name: "Login" });
            } else {
              console.log(response.data.message);
            }
          })
          .catch(function (error) {
            console.log(error);
          });
      });
    },
  },
};
</script>


<style scoped>
.header-row {
  margin: 10px 10px 10px 10px;
}
</style>