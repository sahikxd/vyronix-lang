export default {
  title: "VYRONIX",
  description: "The Future of Systems Programming",
  themeConfig: {
    logo: "/vyronix.ico",
    nav: [
      { text: "Home", link: "/" },
      { text: "Guide", link: "/guide/" },
      { text: "Reference", link: "/reference/" }
    ],
    sidebar: [
      {
        text: "Introduction",
        items: [
          { text: "What is VYRONIX?", link: "/guide/what-is-vyronix" },
          { text: "Getting Started", link: "/guide/getting-started" }
        ]
      },
      {
        text: "Language",
        items: [
          { text: "Syntax", link: "/guide/syntax" },
          { text: "Standard Library", link: "/guide/stdlib" }
        ]
      },
      {
        text: "Architecture",
        items: [
          { text: "VM & Bytecode", link: "/guide/architecture" }
        ]
      }
    ],
    socialLinks: [
      { icon: "github", link: "https://github.com/vyronix-team/vyronix" }
    ]
  }
}
