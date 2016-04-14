Gem::Specification.new do |s|
  s.name = "signer"
  s.version = "0.0.1"
  s.authors = "Brandon Buck"
  s.date = Date.today.to_s
  s.email = "bbuck@thehonestcompany.com"
  s.summary = "A C based request builder and signing gem."
  s.description = s.summary

  s.files = Dir["ext/**/*.{rb,h,c}"]
  # s.test_files = Dir["test/**/*.rb"]
  s.extensions = ["ext/signer/extconf.rb"]
end
